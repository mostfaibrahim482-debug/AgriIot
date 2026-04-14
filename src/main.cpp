#include <Arduino.h>

#include "config.h"
#include "app_state.h"
#include "wifi_manager.h"
#include "time_manager.h"
#include "sensor_manager.h"
#include "firebase_manager.h"
#include "control_manager.h"
#include "safety_manager.h"

static void initHardware()
{
  pinMode(PUMP_PIN, OUTPUT); // GPIO controlling the motor via transistor/MOSFET (output to control pump)
  pinMode(FAN_PIN, OUTPUT);  // GPIO controlling the fan (output to control fan)
  pinMode(LED_PIN, OUTPUT);  // GPIO controlling the LED (output to control LED)

  analogReadResolution(12); // 12-bit ADC resolution (0-4095)
  // ADC_11db attenuation gives a wider input voltage range (up to ~3.3V) which is suitable for our sensors and allows better resolution across the expected range of readings.
  analogSetPinAttenuation(MOISTURE_PIN, ADC_11db); // 0-3.3V range for soil moisture sensor (calibrated for 0-100% range) (0 to 4095 ADC value)
  analogSetPinAttenuation(LDR_PIN, ADC_11db);      // 0-3.3V range for LDR light sensor (calibrated for 0-100% range) (0 to 4095 ADC value)

  dht.begin(); // Initialize DHT sensor

  // Ensure all actuators are off at startup
  setPump(false);
  setFan(false);
  setLed(false);
}

static void handleSetpointFetch(unsigned long now)
{
  if (now - lastFetchMs >= SETPOINT_FETCH_MS)
  {
    lastFetchMs = now;
    fetchCurrentPlantAndSetpoints();
  }
}

static void handleSensorReadAndControl(unsigned long now)
{
  if (now - sensors.lastSensorReadMs >= SENSOR_READ_INTERVAL_MS)
  {
   readSensors();
    updateDailyLightTracking();
    
    controlPump();
    controlFan();
    controlLight();
  }
}

static void handleUploads(unsigned long now)
{
  if (!signupOK)
  {
    return; // Don't attempt uploads if Firebase sign-up wasn't successful
  }
  if (now - lastUploadMs >= FIREBASE_UPLOAD_MS)
  {
    lastUploadMs = now;
    Serial.println("UPLOAD 1: uploadSensorData");
    uploadSensorData();

    Serial.println("UPLOAD 2: uploadActuatorStates");
    uploadActuatorStates();

    Serial.println("UPLOAD 3: uploadSystemStatus");
    uploadSystemStatus();

    Serial.println("UPLOAD 4: uploads done");
  }
}

static void handleStatusPrint(unsigned long now)
{
  if (now - lastStatusPrintMs >= STATUS_PRINT_MS)
  {
    lastStatusPrintMs = now;
    printStatus();
  }
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  initHardware();
  loadFallbackSetpoints();

  connectWiFi();
  if (isWiFiConnected() && waitForInternet(10000))
  {
    initializeTime();

    struct tm timeinfo;
    bool timeReady = getLocalTimeSafe(timeinfo);
    int timeSyncRetries = 0;

    while (!timeReady && timeSyncRetries < 3)
    {
      timeSyncRetries++;
      Serial.printf("Waiting for time sync... attempt %d/3\n", timeSyncRetries);
      delay(2000);
      timeReady = getLocalTimeSafe(timeinfo);
    }

    if (timeReady)
    {
      Serial.println("Time verified synchronized");
      initializeFirebase();
      fetchCurrentPlantAndSetpoints();
    }
    else
    {
      Serial.println("System Time sync failed - Firebase initialization skipped");
    }
  }
  else
  {
    Serial.println("System Starting offline mode");
  }

  initializeWatchdog();

  lastUploadMs = millis();
  lastFetchMs = millis();
  lastStatusPrintMs = millis();
  sensors.lastSensorReadMs = millis();

  Serial.println("System Setup done");
}

void loop()
{
  feedWatchdog();

  maintainWiFi();
  maintainTime();

  unsigned long now = millis();

  handleSetpointFetch(now);
  handleSensorReadAndControl(now);
  handleUploads(now);
  handleStatusPrint(now);

  runSafetyChecks();

  delay(5);
}