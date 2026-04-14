#pragma once

void setPump(bool on);
void setFan(bool on);
void setLed(bool on);

void updateDailyLightTracking();
float getDailyNaturalLightHours();
float getDailySupplementalLightHours();
float getDailyTotalLightHours();

void controlPump();
void controlFan();
void controlLight();
void printStatus();
