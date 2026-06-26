// =====================================================
// config.h - Centralized Configuration
// =====================================================
// All tuneable parameters and pin definitions

#ifndef CONFIG_H
#define CONFIG_H

// ===== WiFi Configuration =====
#include "secrets.h"

// ===== Hardware Pin Definitions =====
#define TRIG_PIN 5
#define ECHO_PIN 18
#define RED_LED 26
#define GREEN_LED 14

// ===== Detection Parameters =====
const float THRESHOLD_CM = 20.0;                    // Distance threshold to trigger "occupied"
const unsigned long MIN_SESSION_SECONDS = 10;       // Minimum duration to record history
const int SENSOR_READINGS = 5;                      // Moving average window size
const int DEBOUNCE_THRESHOLD = 3;                   // Consecutive readings to confirm state change
const unsigned long SENSOR_POLL_MS = 500;           // Sensor poll interval

// ===== History Management =====
#define MAX_HISTORY_RECORDS 100                     // Maximum parking sessions to keep in memory
#define HISTORY_CLEANUP_INTERVAL 60000              // Cleanup old records every 60 seconds

// ===== Time Configuration =====
#define BOOT_EPOCH 1700000000UL                     // Fallback Unix timestamp (Nov 15, 2023)
#define NTP_SERVER "pool.ntp.org"
#define NTP_TIMEZONE 19800                          // IST = UTC+5:30 = 19800 seconds

// ===== Web Server Configuration =====
#define WEB_SERVER_PORT 80
#define AUTO_REFRESH_SECONDS 3

#endif // CONFIG_H
