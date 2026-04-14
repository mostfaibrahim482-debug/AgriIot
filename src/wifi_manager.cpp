#include <WiFi.h>
#include "wifi_manager.h"
#include "config.h"
#include "app_state.h"

bool isWiFiConnected()
{
    bool connected = WiFi.status() == WL_CONNECTED;
    systemStatus.wifi.connected = connected;
    if (connected)
    {
        systemStatus.wifi.statusText = "connected";
        systemStatus.wifi.lastError = "";
    }
    else
    {
        if (systemStatus.wifi.statusText != "connecting")
        {
            systemStatus.wifi.statusText = "disconnected";
            systemStatus.wifi.lastError = "wifi disconnected";
        }
    }
    return connected;
}

void connectWiFi()
{
    if (isWiFiConnected())
        return;

    systemStatus.wifi.connected = false;
    systemStatus.wifi.statusText = "connecting";
    systemStatus.wifi.lastError = "";

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.disconnect(true, true);
    delay(500);

    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print("WiFi Connecting");
    unsigned long startAttempt = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 20000)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (isWiFiConnected())
    {
        systemStatus.wifi.connected = true;
        systemStatus.wifi.statusText = "connected";
        systemStatus.wifi.lastError = "";
        Serial.print("WiFi Connected. IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("DNS: ");
        Serial.println(WiFi.dnsIP());
    }
    else
    {
        systemStatus.wifi.connected = false;
        systemStatus.wifi.statusText = "failed";
        systemStatus.wifi.lastError = "wifi failed to connect";
        Serial.print("WiFi Failed to connect. Status = ");
        Serial.println(WiFi.status());
    }
}

bool waitForInternet(uint32_t timeoutMs)
{
    if (!isWiFiConnected())
    {
        systemStatus.wifi.connected = false;
        systemStatus.wifi.statusText = "disconnected";
        systemStatus.wifi.lastError = "wifi not connected";
        return false;
    }

    IPAddress resolved;
    unsigned long start = millis();

    systemStatus.wifi.statusText = "waiting for dns";
    systemStatus.wifi.lastError = "";

    Serial.print("Waiting for DNS");
    while (millis() - start < timeoutMs)
    {
        if (WiFi.hostByName("google.com", resolved))
        {
            systemStatus.wifi.connected = true;
            systemStatus.wifi.statusText = "dns ready";
            systemStatus.wifi.lastError = "";

            Serial.println();
            Serial.print("DNS OK: ");
            Serial.println(resolved);
            return true;
        }

        Serial.print(".");
        delay(500);
    }
    systemStatus.wifi.connected = true; // WiFi is connected but DNS is not working, so we keep it as connected but with error status
    systemStatus.wifi.statusText = "dns failed";
    systemStatus.wifi.lastError = "dns not ready";

    Serial.println();
    Serial.println("DNS not ready");
    return false;
}

void maintainWiFi()
{
    if (isWiFiConnected())
    {
        systemStatus.wifi.connected = true;
        systemStatus.wifi.statusText = "connected";
        systemStatus.wifi.lastError = "";
        return;
    }
    systemStatus.wifi.connected = false;
    systemStatus.wifi.statusText = "reconnecting";
    systemStatus.wifi.lastError = "wifi disconnected";

    unsigned long now = millis();
    if (now - lastWiFiRetryMs >= WIFI_RETRY_INTERVAL_MS)
    {
        lastWiFiRetryMs = now;
        Serial.println("WiFi Reconnecting...");
        connectWiFi();
    }
}