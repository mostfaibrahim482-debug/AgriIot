#pragma once
#include <time.h>
#include <Arduino.h>

void initializeTime();
void maintainTime();
bool getLocalTimeSafe(struct tm &timeinfo);
bool isTimeValid();
bool isDaytime(const struct tm &timeinfo);
bool isAfterSunset(const struct tm &timeinfo);
String getFormattedLocalTime();