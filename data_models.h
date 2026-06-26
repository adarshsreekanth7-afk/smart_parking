// =====================================================
// data_models.h - Parking Session Data Structures
// =====================================================
// Defines ParkingSession struct and history management

#ifndef DATA_MODELS_H
#define DATA_MODELS_H

#include <vector>
#include "config.h"
#include "timing.h"

// ===== Parking Session Data Structure =====
struct ParkingSession {
    int carID;
    unsigned long entryEpoch;   // Unix timestamp when car arrived
    unsigned long exitEpoch;    // Unix timestamp when car left
    unsigned long duration;     // Session duration in seconds
};

// ===== History Management =====
std::vector<ParkingSession> parkingHistory;
unsigned long lastCleanupTime = 0;

// ===== Add Session to History =====
// Automatically manages max history size
void addSessionToHistory(const ParkingSession& session) {
    // Remove oldest session if history is full
    if (parkingHistory.size() >= MAX_HISTORY_RECORDS) {
        parkingHistory.erase(parkingHistory.begin());
        Serial.println("[HISTORY] Removed oldest record (limit reached)");
    }
    
    parkingHistory.push_back(session);
    Serial.println("[HISTORY] Session #" + String(session.carID) + 
                   " recorded: " + String(session.duration) + "s");
}

// ===== Get Session Count =====
int getSessionCount() {
    return parkingHistory.size();
}

// ===== Get Recent Sessions (for web display) =====
// Returns the most recent N sessions
std::vector<ParkingSession> getRecentSessions(int count = 15) {
    std::vector<ParkingSession> recent;
    
    int start = max(0, (int)parkingHistory.size() - count);
    for (int i = parkingHistory.size() - 1; i >= start; i--) {
        recent.push_back(parkingHistory[i]);
    }
    
    return recent;
}

#endif // DATA_MODELS_H
