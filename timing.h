// =====================================================
// timing.h - Time & Duration Formatting Utilities
// =====================================================
// Converts Unix epochs to human-readable timestamps
// and formats duration in a friendly way (e.g., "2h 15m 32s")

#ifndef TIMING_H
#define TIMING_H

#include <time.h>
#include "config.h"

// ===== Global Time State =====
// bootEpoch: Unix timestamp offset added to millis() for real time
// In production, set this via NTP sync or web endpoint
unsigned long bootEpoch = BOOT_EPOCH;

// ===== Get Current Unix Timestamp =====
// Returns current Unix epoch based on bootEpoch offset
unsigned long getEpochNow() {
    return bootEpoch + (millis() / 1000);
}

// ===== Format Unix Timestamp to Human-Readable Time =====
// Input: Unix epoch (seconds since 1970-01-01 00:00:00)
// Output: String in format "DD/MM/YYYY HH:MM:SS"
String formatRealTime(unsigned long epochSec) {
    time_t t = (time_t)epochSec;
    struct tm* tmInfo = localtime(&t);
    char buf[30];
    strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", tmInfo);
    return String(buf);
}

// ===== Format Duration in Seconds =====
// Input: Duration in seconds
// Output: Human-friendly string (e.g., "45 sec", "3m 22s", "1h 45m 12s")
String formatDuration(unsigned long secs) {
    if (secs < 60) {
        return String(secs) + " sec";
    }
    
    unsigned long mins = secs / 60;
    unsigned long rem = secs % 60;
    
    if (mins < 60) {
        return String(mins) + "m " + String(rem) + "s";
    }
    
    unsigned long hrs = mins / 60;
    mins = mins % 60;
    return String(hrs) + "h " + String(mins) + "m " + String(rem) + "s";
}

// ===== Set Boot Epoch (for NTP sync or manual time set) =====
// Call this when you have accurate time (e.g., from NTP)
void setBootEpoch(unsigned long currentUnixTime) {
    bootEpoch = currentUnixTime - (millis() / 1000);
}

#endif // TIMING_H
