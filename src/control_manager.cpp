#include <Arduino.h>
#include "control_manager.h"
#include "config.h"
#include "app_state.h"
#include "utils.h"
#include "time_manager.h"

static void applyActuatorPin(uint8_t pin, bool logicalOn, bool inverted)
{
    bool pinLevel = inverted ? !logicalOn : logicalOn;
    digitalWrite(pin, pinLevel ? HIGH : LOW);
}

static int getLightCycleDayKey(const struct tm &timeinfo)
{
    int dayKey = timeinfo.tm_yday;
    if (timeinfo.tm_hour < DAY_START_HOUR)
    {
        dayKey--;
        if (dayKey < 0)
        {
            dayKey = 365;
        }
    }
    return dayKey;
}

void setPump(bool on)
{
    if (on)
    {
        pumpSafety.pumpStartMs = millis();
        pumpSafety.timingActive = true;
    }
    else
    {
        if (actuators.pump)
        {
            pumpSafety.lastPumpOffMs = millis();
        }
        pumpSafety.timingActive = false;
    }

    actuators.pump = on;
    applyActuatorPin(PUMP_PIN, on, PUMP_INVERTED);
}

void setFan(bool on)
{
    actuators.fan = on;
    applyActuatorPin(FAN_PIN, on, FAN_INVERTED);
}

void setLed(bool on)
{
    actuators.led = on;
    applyActuatorPin(LED_PIN, on, LED_INVERTED);
}

static void resetLightTrackerIfNewDay(const struct tm &timeinfo)
{
    int cycleDayKey = getLightCycleDayKey(timeinfo);
    if (lightTracker.lastDayOfYear != cycleDayKey)
    {
        lightTracker.lastDayOfYear = cycleDayKey;
        lightTracker.dailyNaturalLightSeconds = 0;
        lightTracker.dailySupplementalLightSeconds = 0;
        lightTracker.lastUpdateMs = millis();
        Serial.println("Light New sunrise-based cycle detected, counters reset");
    }
}

void updateDailyLightTracking()
{
    struct tm timeinfo;
    if (!getLocalTimeSafe(timeinfo))
    {
        lightTracker.timeValid = false;
        return;
    }

    lightTracker.timeValid = true;
    resetLightTrackerIfNewDay(timeinfo);

    unsigned long nowMs = millis();
    if (lightTracker.lastUpdateMs == 0)
    {
        lightTracker.lastUpdateMs = nowMs;
        return;
    }

    unsigned long elapsedSec = (nowMs - lightTracker.lastUpdateMs) / 1000;
    lightTracker.lastUpdateMs = nowMs;

    if (elapsedSec == 0)
        return;

    if (isDaytime(timeinfo) && sensors.lightPercent >= SUNLIGHT_THRESHOLD_PERCENT)
    {
        lightTracker.dailyNaturalLightSeconds += elapsedSec;
    }

    if (actuators.led)
    {
        lightTracker.dailySupplementalLightSeconds += elapsedSec;
    }
}

float getDailyNaturalLightHours()
{
    return lightTracker.dailyNaturalLightSeconds / 3600.0f;
}

float getDailySupplementalLightHours()
{
    return lightTracker.dailySupplementalLightSeconds / 3600.0f;
}

float getDailyTotalLightHours()
{
    return (lightTracker.dailyNaturalLightSeconds + lightTracker.dailySupplementalLightSeconds) / 3600.0f;
}

static bool canStartPump()
{
    return (millis() - pumpSafety.lastPumpOffMs) >= PUMP_COOLDOWN_MS;
}

void controlPump()
{
    if (isnan(sensors.soilMoisturePercent))
        return;

    if (!actuators.pump && sensors.soilMoisturePercent < setpoints.moisture.min && canStartPump())
    {
        setPump(true);
    }
    else if (actuators.pump && sensors.soilMoisturePercent >= setpoints.moisture.max)
    {
        setPump(false);
    }
}

void controlFan()
{
    if (!sensors.validDHT)
        return;

    bool needFan = (sensors.temperatureC > setpoints.temperature.max);
    bool safeToTurnOff = (sensors.temperatureC <= (setpoints.temperature.max - FAN_TEMP_OFF_MARGIN));
    if (!actuators.fan && needFan)
    {
        setFan(true);
    }
    else if (actuators.fan && safeToTurnOff)
    {
        setFan(false);
    }
}

void controlLight()
{
    if (isnan(sensors.lightPercent))
        return;

    struct tm timeinfo;
    if (!getLocalTimeSafe(timeinfo))
    {
        setLed(false);
        return;
    }

    float totalHours = getDailyTotalLightHours();
    bool sunlightEnoughNow = sensors.lightPercent >= SUNLIGHT_THRESHOLD_PERCENT;

    if (isDaytime(timeinfo))
    {
        if (sunlightEnoughNow)
        {
            setLed(false);
        }
        else
        {
            setLed(true);
        }
        return;
    }

    if (isAfterSunset(timeinfo))
    {
        if (totalHours < setpoints.lightHours.min)
        {
            setLed(true);
        }
        else
        {
            setLed(false);
        }
        return;
    }

    // before sunrise: keep the previous light cycle alive until the required minimum is reached
    if (totalHours < setpoints.lightHours.min)
    {
        setLed(true);
    }
    else
    {
        setLed(false);
    }
}

void printStatus()
{
    Serial.println("\n================ STATUS ================");
    Serial.printf("Time: %s\n", getFormattedLocalTime().c_str());
    Serial.printf("Plant: %s\n", setpoints.plantName.c_str());

    Serial.printf("Temp Range: %.1f -> %.1f C\n", setpoints.temperature.min, setpoints.temperature.max);
    Serial.printf("Soil Range: %.1f -> %.1f %%\n", setpoints.moisture.min, setpoints.moisture.max);
    Serial.printf("Light Hours Range: %.1f -> %.1f h\n", setpoints.lightHours.min, setpoints.lightHours.max);

    if (sensors.validDHT)
    {
        Serial.printf("Temperature: %.2f C\n", sensors.temperatureC);
    }
    else
    {
        Serial.println("Temperature: DHT invalid or stale");
    }

    Serial.printf("Soil Raw: %d\n", sensors.soilRaw);
    Serial.printf("Soil Moisture: %.2f %%\n", sensors.soilMoisturePercent);
    Serial.printf("LDR Raw: %d\n", sensors.ldrRaw);
    Serial.printf("Light Percent: %.2f %%\n", sensors.lightPercent);

    Serial.printf("Natural Light Today: %.2f h\n", getDailyNaturalLightHours());
    Serial.printf("Supplemental Light Today: %.2f h\n", getDailySupplementalLightHours());
    Serial.printf("Total Light Today: %.2f h\n", getDailyTotalLightHours());

    Serial.printf("Pump: %s\n", actuators.pump ? "ON" : "OFF");
    Serial.printf("Fan : %s\n", actuators.fan ? "ON" : "OFF");
    Serial.printf("LED : %s\n", actuators.led ? "ON" : "OFF");
    Serial.println("========================================\n");
}