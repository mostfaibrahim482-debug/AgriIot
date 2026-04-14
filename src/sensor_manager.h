#pragma once

void loadFallbackSetpoints();
void readSensors();

float getSoilMoisturePercent(int rawValue);
float getLightPercent(int rawValue);