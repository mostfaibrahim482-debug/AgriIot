#pragma once

void initializeFirebase();
bool fetchCurrentPlantAndSetpoints();
void uploadSensorData();
void uploadActuatorStates();
void uploadSystemStatus();