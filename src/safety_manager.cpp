#include <Arduino.h>
#include <esp_task_wdt.h>
#include "safety_manager.h"
#include "config.h"
#include "app_state.h"
#include "control_manager.h"

void initializeWatchdog()
{
  esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
  esp_task_wdt_add(NULL);
  Serial.println("Safety Watchdog initialized");
}

void feedWatchdog()
{
  esp_task_wdt_reset();
}

void runSafetyChecks()
{
  // Pump max runtime fail-safe
  // if (actuators.pump && pumpSafety.timingActive)
  // {
  //   if (millis() - pumpSafety.pumpStartMs >= MAX_PUMP_ON_MS)
  //   {
  //     Serial.println("Safety Pump max runtime exceeded. Turning pump OFF.");
  //     setPump(false);
  //   }
  // }

  // Sensor stale fail-safe
  // if (millis() - sensors.lastSensorReadMs > SENSOR_STALE_TIMEOUT_MS)
  // {
  //   if (actuators.pump)
  //   {
  //     Serial.println("Safety Sensors stale. Turning pump OFF.");
  //     setPump(false);
  //   }
  // }

  // DHT critical timeout fail-safe
  if (!sensors.validDHT && (millis() - sensors.lastValidDhtMs > DHT_CRITICAL_TIMEOUT_MS))
  {
    Serial.println("Safety DHT critically stale. Turning fan ON as fail-safe.");
    setFan(true);
  }
}