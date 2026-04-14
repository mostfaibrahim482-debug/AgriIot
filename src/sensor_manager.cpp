#include <Arduino.h>
#include <math.h>
#include "sensor_manager.h"
#include "config.h"
#include "app_state.h"
#include "utils.h"

static int applyCleaning(uint8_t pin, EmaFilterState &filterState)
{
    int samples[ADC_SAMPLES];
    int absDeviations[ADC_SAMPLES];

    for (uint8_t i = 0; i < ADC_SAMPLES; i++)
    {
        samples[i] = analogRead(pin);
        delay(ADC_SAMPLE_DELAY_MS); // small delay between samples to allow for ADC stabilization
    }

    insertionSortInt(samples, ADC_SAMPLES);
    int median = medianOfSortedInt(samples, ADC_SAMPLES);

    for (uint8_t i = 0; i < ADC_SAMPLES; i++)
    {
        absDeviations[i] = abs(samples[i] - median);
    }

    insertionSortInt(absDeviations, ADC_SAMPLES);
    int mad = medianOfSortedInt(absDeviations, ADC_SAMPLES);

    long sum = 0;
    int count = 0;

    if (mad == 0)
    {
        for (uint8_t i = 0; i < ADC_SAMPLES; i++)
        {
            sum += samples[i];
            count++;
        }
    }
    else
    {
        float threshold = MAD_SCALE_THRESHOLD * mad;
        for (uint8_t i = 0; i < ADC_SAMPLES; i++)
        {
            if (abs(samples[i] - median) <= threshold)
            {
                sum += samples[i];
                count++;
            }
        }
    }

    float cleaned = (count > 0) ? ((float)sum / count) : (float)median;

    if (!filterState.initialized)
    {
        filterState.initialized = true;
        filterState.value = cleaned;
    }
    else
    {
        filterState.value = (ADC_EMA_ALPHA * cleaned) + ((1.0f - ADC_EMA_ALPHA) * filterState.value);
    }

    return (int)roundf(filterState.value);
}

static void readDHTSafely()
{
    float t = dht.readTemperature();

    if (isnan(t))
    {
        sensors.dhtFailCount++;

        systemStatus.dht.valid = false;
        systemStatus.dht.failCount = sensors.dhtFailCount;
        systemStatus.dht.lastError = "DHT temperature read failed";

        if (millis() - sensors.lastValidDhtMs > DHT_STALE_TIMEOUT_MS)
        {
            sensors.validDHT = false;
            systemStatus.dht.lastError = "DHT data timeout";
        }

        if (sensors.dhtFailCount >= DHT_MAX_FAILS_BEFORE_REINIT)
        {
            Serial.println("DHT Too many failures, reinitializing...");
            dht.begin();
            sensors.dhtFailCount = 0;

            systemStatus.dht.failCount = 0;
            systemStatus.dht.lastError = "DHT reinitialized after failures";
        }
        return;
    }

    sensors.temperatureC = t;
    sensors.validDHT = true;
    sensors.dhtFailCount = 0;
    sensors.lastValidDhtMs = millis();

    systemStatus.dht.valid = true;
    systemStatus.dht.failCount = 0;
    systemStatus.dht.lastError = "";
}

void loadFallbackSetpoints()
{
    setpoints.plantName = DEFAULT_PLANT_NAME;
    setpoints.temperature = {FALLBACK_TEMP_MIN, FALLBACK_TEMP_MAX};
    setpoints.moisture = {FALLBACK_SOIL_MIN, FALLBACK_SOIL_MAX};
    setpoints.lightHours = {FALLBACK_LIGHT_HOURS_MIN, FALLBACK_LIGHT_HOURS_MAX};
}

float getSoilMoisturePercent(int rawValue)
{
    float percent = mapFloat((float)rawValue, (float)SOIL_DRY_VALUE, (float)SOIL_WET_VALUE, 0.0f, 100.0f);
    return clampValue(percent, 0.0f, 100.0f);
}

float getLightPercent(int rawValue)
{
    float percent = mapFloat((float)rawValue, (float)LDR_DARK_VALUE, (float)LDR_BRIGHT_VALUE, 0.0f, 100.0f);
    return clampValue(percent, 0.0f, 100.0f);
}

void readSensors()
{
    sensors.soilRaw = applyCleaning(MOISTURE_PIN, soilFilterState);
    sensors.ldrRaw = applyCleaning(LDR_PIN, ldrFilterState);

    sensors.soilMoisturePercent = getSoilMoisturePercent(sensors.soilRaw);
    sensors.lightPercent = getLightPercent(sensors.ldrRaw);

    readDHTSafely();

    sensors.lastSensorReadMs = millis();
}