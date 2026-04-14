#include "app_state.h"
#include "config.h"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig firebaseConfig;
DHT dht(DHT_PIN, DHT_TYPE);

PlantSetpoint setpoints;
SensorData sensors;
ActuatorData actuators;
LightTracker lightTracker;
PumpSafetyState pumpSafety;

EmaFilterState soilFilterState;
EmaFilterState ldrFilterState;

SystemStatusData systemStatus;

bool signupOK = false;

unsigned long lastUploadMs = 0;
unsigned long lastFetchMs = 0;
unsigned long lastStatusPrintMs = 0;
unsigned long lastWiFiRetryMs = 0;
unsigned long lastTimeSyncAttemptMs = 0;