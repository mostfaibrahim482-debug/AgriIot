#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include "app_state.h"
#include "firebase_manager.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "config.h"
#include "control_manager.h"
#include "time_manager.h"

static bool getFloatValue(const String &path, float &outValue)
{
    if (Firebase.RTDB.getFloat(&fbdo, path.c_str()))
    {
        outValue = fbdo.floatData();
        return true;
    }

    Serial.printf("Firebase getFloat failed %s => %s\n", path.c_str(), fbdo.errorReason().c_str());
    return false;
}

static bool getStringValue(const String &path, String &outValue)
{
    if (Firebase.RTDB.getString(&fbdo, path.c_str()))
    {
        outValue = fbdo.stringData();
        return true;
    }

    Serial.printf("Firebase getString failed %s => %s\n", path.c_str(), fbdo.errorReason().c_str());
    return false;
}

void initializeFirebase()
{
    signupOK = false;
    systemStatus.firebase.signupOK = false;
    systemStatus.firebase.lastError = "";

    firebaseConfig.api_key = API_KEY;
    firebaseConfig.database_url = DATABASE_URL;
    firebaseConfig.token_status_callback = tokenStatusCallback;

    Serial.println("Starting Firebase authentication...");

    if (Firebase.signUp(&firebaseConfig, &auth, "", ""))
    {
        systemStatus.firebase.signupOK = true;
        systemStatus.firebase.lastError = "";
        signupOK = true;
        Serial.println("Firebase anonymous signUp OK");
    }
    else
    {
        systemStatus.firebase.signupOK = false;
        systemStatus.firebase.lastError = String(firebaseConfig.signer.signupError.message.c_str());
        signupOK = false;
        Serial.printf("Firebase signUp failed: %s\n", firebaseConfig.signer.signupError.message.c_str());
        return;
    }

    Firebase.begin(&firebaseConfig, &auth);
    Firebase.reconnectWiFi(true);

    Serial.println("Firebase begin OK");
}

bool fetchCurrentPlantAndSetpoints()
{
    if (!Firebase.ready() || !signupOK)
    {
        Serial.println("Config Firebase not ready, using existing/fallback setpoints");
        return false;
    }

    String currentPlant;
    if (!getStringValue("/currentPlant", currentPlant) || currentPlant.length() == 0)
    {
        Serial.println("Config Could not fetch currentPlant, keeping previous setpoints");
        return false;
    }

    PlantSetpoint tempSetpoints;
    tempSetpoints.plantName = currentPlant;

    String base = "/plants/" + currentPlant;

    bool ok = true;
    ok &= getFloatValue(base + "/temperature/min", tempSetpoints.temperature.min);
    ok &= getFloatValue(base + "/temperature/max", tempSetpoints.temperature.max);
    ok &= getFloatValue(base + "/moisture/min", tempSetpoints.moisture.min);
    ok &= getFloatValue(base + "/moisture/max", tempSetpoints.moisture.max);
    ok &= getFloatValue(base + "/lightHours/min", tempSetpoints.lightHours.min);
    ok &= getFloatValue(base + "/lightHours/max", tempSetpoints.lightHours.max);

    if (!ok)
    {
        Serial.println("Config Failed to fetch all setpoints, keeping previous values");
        return false;
    }

    setpoints = tempSetpoints;
    Serial.printf("Config Plant loaded: %s\n", setpoints.plantName.c_str());
    return true;
}

void uploadSensorData()
{
    if (!Firebase.ready() || !signupOK)
        return;

    bool ok = true;

    ok &= Firebase.RTDB.setFloat(&fbdo, "/sensors/soilMoisture", sensors.soilMoisturePercent);
    ok &= Firebase.RTDB.setFloat(&fbdo, "/sensors/lightPercent", sensors.lightPercent);
    ok &= Firebase.RTDB.setInt(&fbdo, "/sensors/soilRaw", sensors.soilRaw);
    ok &= Firebase.RTDB.setInt(&fbdo, "/sensors/ldrRaw", sensors.ldrRaw);

    if (sensors.validDHT)
    {
        ok &= Firebase.RTDB.setFloat(&fbdo, "/sensors/temperature", sensors.temperatureC);
    }

    ok &= Firebase.RTDB.setFloat(&fbdo, "/sensors/dailyNaturalLightHours", getDailyNaturalLightHours());
    ok &= Firebase.RTDB.setFloat(&fbdo, "/sensors/dailySupplementalLightHours", getDailySupplementalLightHours());
    ok &= Firebase.RTDB.setFloat(&fbdo, "/sensors/dailyTotalLightHours", getDailyTotalLightHours());
    ok &= Firebase.RTDB.setBool(&fbdo, "/sensors/timeValid", lightTracker.timeValid);

    if (!ok)
    {
        Serial.printf("Firebase Sensor upload failed: %s\n", fbdo.errorReason().c_str());
    }
}

static String safeFirebaseString(const String &value, const char *fallback = "none")
{
    return value.length() ? value : String(fallback);
}

void uploadSystemStatus()
{
    if (!Firebase.ready() || !signupOK)
        return;

    bool ok = true;

    String wifiStatusText = safeFirebaseString(systemStatus.wifi.statusText, "unknown");
    String wifiLastError = safeFirebaseString(systemStatus.wifi.lastError, "none");
    String firebaseLastError = safeFirebaseString(systemStatus.firebase.lastError, "none");
    String timeFormatted = safeFirebaseString(getFormattedLocalTime(), "NTP not ready");
    String dhtLastError = safeFirebaseString(systemStatus.dht.lastError, "none");
    String alertSeverity = safeFirebaseString(systemStatus.alert.severity, "info");
    String alertTitle = safeFirebaseString(systemStatus.alert.title, "none");
    String alertMessage = safeFirebaseString(systemStatus.alert.message, "none");
    String alertTimestamp = safeFirebaseString(systemStatus.alert.timestamp, "none");

    Serial.println("SYS 1");
    if (!(ok &= Firebase.RTDB.setBool(&fbdo, "/system/wifi/connected", systemStatus.wifi.connected)))
        Serial.printf("Failed at SYS 1: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 2");
    if (!(ok &= Firebase.RTDB.setString(&fbdo, "/system/wifi/statusText", wifiStatusText.c_str())))
        Serial.printf("Failed at SYS 2: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 3");
    if (!(ok &= Firebase.RTDB.setString(&fbdo, "/system/wifi/lastError", wifiLastError.c_str())))
        Serial.printf("Failed at SYS 3: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 4");
    if (!(ok &= Firebase.RTDB.setBool(&fbdo, "/system/firebase/ready", Firebase.ready())))
        Serial.printf("Failed at SYS 4: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 5");
    if (!(ok &= Firebase.RTDB.setBool(&fbdo, "/system/firebase/signupOK", signupOK)))
        Serial.printf("Failed at SYS 5: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 6");
    if (!(ok &= Firebase.RTDB.setString(&fbdo, "/system/firebase/lastError", firebaseLastError.c_str())))
        Serial.printf("Failed at SYS 6: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 7");
    if (!(ok &= Firebase.RTDB.setBool(&fbdo, "/system/time/valid", isTimeValid())))
        Serial.printf("Failed at SYS 7: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 8");
    if (!(ok &= Firebase.RTDB.setString(&fbdo, "/system/time/formatted", timeFormatted.c_str())))
        Serial.printf("Failed at SYS 8: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 9");
    if (!(ok &= Firebase.RTDB.setBool(&fbdo, "/system/dht/valid", sensors.validDHT)))
        Serial.printf("Failed at SYS 9: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 10");
    if (!(ok &= Firebase.RTDB.setInt(&fbdo, "/system/dht/failCount", sensors.dhtFailCount)))
        Serial.printf("Failed at SYS 10: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 11");
    if (!(ok &= Firebase.RTDB.setString(&fbdo, "/system/dht/lastError", dhtLastError.c_str())))
        Serial.printf("Failed at SYS 11: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 12");
    if (!(ok &= Firebase.RTDB.setBool(&fbdo, "/system/alert/visible", systemStatus.alert.visible)))
        Serial.printf("Failed at SYS 12: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 13");
    if (!(ok &= Firebase.RTDB.setString(&fbdo, "/system/alert/severity", alertSeverity.c_str())))
        Serial.printf("Failed at SYS 13: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 14");
    if (!(ok &= Firebase.RTDB.setString(&fbdo, "/system/alert/title", alertTitle.c_str())))
        Serial.printf("Failed at SYS 14: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 15");
    if (!(ok &= Firebase.RTDB.setString(&fbdo, "/system/alert/message", alertMessage.c_str())))
        Serial.printf("Failed at SYS 15: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 16");
    if (!(ok &= Firebase.RTDB.setString(&fbdo, "/system/alert/timestamp", alertTimestamp.c_str())))
        Serial.printf("Failed at SYS 16: %s\n", fbdo.errorReason().c_str());

    Serial.println("SYS 17");

    if (!ok)
    {
        Serial.printf("Firebase System status upload failed: %s\n", fbdo.errorReason().c_str());
    }
    else
    {
        Serial.println("Firebase System status upload OK");
    }
}

void uploadActuatorStates()
{
    if (!Firebase.ready() || !signupOK)
        return;

    bool ok = true;
    ok &= Firebase.RTDB.setBool(&fbdo, "/actuators/pump", actuators.pump);
    ok &= Firebase.RTDB.setBool(&fbdo, "/actuators/fan", actuators.fan);
    ok &= Firebase.RTDB.setBool(&fbdo, "/actuators/led", actuators.led);

    if (!ok)
    {
        Serial.printf("Firebase Actuator upload failed: %s\n", fbdo.errorReason().c_str());
    }
}