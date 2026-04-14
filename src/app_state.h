#pragma once
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include "types.h"
#include "config.h"

extern FirebaseData fbdo;             // Firebase data object to store received data
extern FirebaseAuth auth;             // Firebase auth object to store user credentials
extern FirebaseConfig firebaseConfig; // Firebase config object to store Firebase project configuration
extern DHT dht;                       // DHT sensor object

extern PlantSetpoint setpoints;    // Struct to store plant setpoints
extern SensorData sensors;         // Struct to store sensor data
extern ActuatorData actuators;     // Struct to store actuator data
extern LightTracker lightTracker;  // Struct to track light hours
extern PumpSafetyState pumpSafety; // Struct to track pump safety

extern EmaFilterState soilFilterState; // Soil moisture filter state
extern EmaFilterState ldrFilterState;  // LDR filter state

extern SystemStatusData systemStatus; // Struct to store overall system status, including WiFi, time sync status, dht status, firebase status, and alert status

extern bool signupOK; // variable to store if sign up was successful

extern unsigned long lastUploadMs;          // variable to store the last time sensor data was uploaded
extern unsigned long lastFetchMs;           // variable to store the last time sensor data was fetched
extern unsigned long lastStatusPrintMs;     // variable to store the last time status was printed
extern unsigned long lastWiFiRetryMs;       // variable to store the last time WiFi connection was retried
extern unsigned long lastTimeSyncAttemptMs; // variable to store the last time WiFi connection was retried