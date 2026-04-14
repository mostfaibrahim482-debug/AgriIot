#pragma once
#include <Arduino.h>

struct Range
{
    float min;
    float max;
};

struct PlantSetpoint
{
    String plantName;
    Range temperature;
    Range moisture;
    Range lightHours;
};

struct SensorData
{
    float temperatureC = NAN;
    float soilMoisturePercent = NAN;
    float lightPercent = NAN;

    int soilRaw = 0;
    int ldrRaw = 0;

    bool validDHT = false;
    uint8_t dhtFailCount = 0;
    unsigned long lastValidDhtMs = 0;
    unsigned long lastSensorReadMs = 0;
};

struct ActuatorData
{
    bool pump = false;
    bool fan = false;
    bool led = false;
};

struct LightTracker
{
    unsigned long lastUpdateMs = 0;
    unsigned long dailyNaturalLightSeconds = 0;
    unsigned long dailySupplementalLightSeconds = 0;
    int lastDayOfYear = -1;
    bool timeValid = false;
};

struct EmaFilterState
{
    bool initialized = false;
    float value = 0.0f;
};

struct PumpSafetyState
{
    unsigned long pumpStartMs = 0;
    unsigned long lastPumpOffMs = 0;
    bool timingActive = false;
};

struct WiFiStatusData
{
    bool connected = false;
    String statusText = "idle";
    String lastError = "";
};

struct FirebaseStatusData
{
    bool ready = false;
    bool signupOK = false;
    String lastError = "";
};

struct TimeStatusData
{
    bool valid = false;
    String formatted = "";
    String lastError = "";
};

struct DhtStatusData
{
    bool valid = false;
    uint8_t failCount = 0;
    String lastError = "";
};

struct AlertData
{
    bool visible = false;
    String severity = "info";
    String title = "";
    String message = "";
    String timestamp = "";
};

struct SystemStatusData
{
    WiFiStatusData wifi;
    FirebaseStatusData firebase;
    TimeStatusData time;
    DhtStatusData dht;
    AlertData alert;
};