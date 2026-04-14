#pragma once
#include <Arduino.h>

#define WIFI_SSID "Eskandar"
#define WIFI_PASS "alexandar221196_."

#define API_KEY "AIzaSyCL32s5EY1OG-67iGy3M2dMZ8ZmuKQqjHM"
#define DATABASE_URL "https://agriiot-e6499-default-rtdb.europe-west1.firebasedatabase.app/"

// Digital Input/Output pins: 32, 33, 25, 26, 27, 14, 12, 13, 23, 22, 21, 19, 18, 5, 17, 16, 4, 0, 2, 15
// Analog Input/Output (ADC1 + ADC2): 32, 33, 25, 26, 27, 14, 12, 13
// Analog Input only (ADC1 - Input ): 34, 35, 36(VP), 39(VN)

#define MOISTURE_PIN 32 // AO Moisture sensor ADC (Analog-to-Digital Converter).
#define LDR_PIN 34      // LDR Light sensor ADC (Analog-to-Digital Converter).
#define DHT_PIN 18      // DHT22 Temperature and Humidity sensor digital pin
#define DHT_TYPE DHT22

#define PUMP_PIN 13 // GPIO controlling the motor via transistor/MOSFET
#define FAN_PIN 14  // GPIO controlling the fan
#define LED_PIN 27  // GPIO controlling the LED

static const int ADC_MAX_VALUE = 4095;

// Soil sensor calibration
static const int SOIL_DRY_VALUE = 3200; // 0%
static const int SOIL_WET_VALUE = 1400; // 100%

// LDR calibration
static const int LDR_DARK_VALUE = 4095;   // very dark
static const int LDR_BRIGHT_VALUE = 1200; // bright enough, tune experimentally

// Timing intervals (ms)
static const unsigned long SENSOR_READ_INTERVAL_MS = 2500;
static const unsigned long FIREBASE_UPLOAD_MS = 5000;
static const unsigned long SETPOINT_FETCH_MS = 15000;
static const unsigned long STATUS_PRINT_MS = 5000;
static const unsigned long WIFI_RETRY_INTERVAL_MS = 5000;
static const unsigned long TIME_SYNC_RETRY_MS = 30000;

// Timing intervals (hours) for control
static const int DAY_START_HOUR = 6; // 06:00 local time
static const int SUNSET_HOUR = 18;   // 18:00 local time

// Control tuning
static const float FAN_TEMP_OFF_MARGIN = 1.0f; // hysteresis
static const float SUNLIGHT_THRESHOLD_PERCENT = 35.0f;

// fallback if Firebase unavailable
static const char *DEFAULT_PLANT_NAME = "Basil";

static const float FALLBACK_TEMP_MIN = 21.0f;
static const float FALLBACK_TEMP_MAX = 29.0f;
static const float FALLBACK_SOIL_MIN = 60.0f;
static const float FALLBACK_SOIL_MAX = 80.0f;
static const float FALLBACK_LIGHT_HOURS_MIN = 6.0f;
static const float FALLBACK_LIGHT_HOURS_MAX = 8.0f;

// Actuator electrical behavior
static const bool PUMP_INVERTED = true;
static const bool FAN_INVERTED = true;
static const bool LED_INVERTED = true;

// ADC tuning parameters
static const uint8_t ADC_SAMPLES = 15;         // number of samples to average
static const uint16_t ADC_SAMPLE_DELAY_MS = 5; // small settle delay instead of long blocking delay
static const float ADC_EMA_ALPHA = 0.25f;      // exponential moving average (0..1)
static const float MAD_SCALE_THRESHOLD = 2.5f; // threshold for median absolute deviation

// DHT recovery
static const uint8_t DHT_MAX_FAILS_BEFORE_REINIT = 5;        // number of fails before reinitializing
static const unsigned long DHT_STALE_TIMEOUT_MS = 15000;     // keep last good reading for 15s
static const unsigned long DHT_CRITICAL_TIMEOUT_MS = 300000; // 5 min

// Safety / Watchdog
static const unsigned long MAX_PUMP_ON_MS = 15000;          // 15 sec max continuous
static const unsigned long PUMP_COOLDOWN_MS = 10000;        // 10 sec cooldown
static const unsigned long SENSOR_STALE_TIMEOUT_MS = 30000; // 30 sec stale sensors fail-safe
static const unsigned long WATCHDOG_TIMEOUT_S = 20;         // 20 sec watchdog timeout fail-safe

// NTP
static const char *NTP_SERVER_1 = "pool.ntp.org";
static const char *NTP_SERVER_2 = "time.nist.gov";
static const char *NTP_SERVER_3 = "time.google.com";

// Egypt / Cairo TZ
static const char *TZ_INFO = "EET-2EEST,M4.5.5/0,M10.5.5/0";