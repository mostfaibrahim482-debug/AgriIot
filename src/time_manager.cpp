#include <time.h>
#include "time_manager.h"
#include "config.h"
#include "app_state.h"
#include "wifi_manager.h"

static bool timeInitialized = false;

void initializeTime()
{
    setenv("TZ", TZ_INFO, 1);
    tzset();

    systemStatus.time.valid = false;
    systemStatus.time.formatted = "NTP initializing...";
    systemStatus.time.lastError = "";

    configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10000))
    {
        timeInitialized = true;

        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

        systemStatus.time.valid = true;
        systemStatus.time.formatted = String(buffer);
        systemStatus.time.lastError = "";
    }
    else
    {
        timeInitialized = false;
        systemStatus.time.valid = false;
        systemStatus.time.formatted = "NTP not ready";
        systemStatus.time.lastError = "initial NTP sync failed";
    }
}

void maintainTime()
{
    if (!isWiFiConnected())
    {
        systemStatus.time.valid = false;
        systemStatus.time.formatted = "NTP not ready";
        systemStatus.time.lastError = "wifi not connected";
        return;
    }

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100))
    {
        timeInitialized = true;
        systemStatus.time.valid = true;

        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        systemStatus.time.formatted = String(buffer);
        systemStatus.time.lastError = "";
        return;
    }

    systemStatus.time.valid = false;
    systemStatus.time.formatted = "NTP not ready";
    systemStatus.time.lastError = "NTP sync failed";

    unsigned long now = millis();
    if (now - lastTimeSyncAttemptMs >= TIME_SYNC_RETRY_MS)
    {
        lastTimeSyncAttemptMs = now;
        configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
    }
}

bool getLocalTimeSafe(struct tm &timeinfo)
{
    return getLocalTime(&timeinfo, 100);
}

bool isTimeValid()
{
    if (!timeInitialized)
        return false;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100))
        return false;

    return (timeinfo.tm_year + 1900) >= 2024;
}

bool isDaytime(const struct tm &timeinfo)
{
    return timeinfo.tm_hour >= DAY_START_HOUR && timeinfo.tm_hour < SUNSET_HOUR;
}

bool isAfterSunset(const struct tm &timeinfo)
{
    return timeinfo.tm_hour >= SUNSET_HOUR;
}

String getFormattedLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTimeSafe(timeinfo))
        return "NTP not ready";

    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buffer);
}