#pragma once

bool isWiFiConnected();
void connectWiFi();
void maintainWiFi();
bool waitForInternet(uint32_t timeoutMs = 10000);