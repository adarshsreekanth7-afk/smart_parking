// =====================================================
// state_machine.h - Parking Slot State Management
// =====================================================
// Handles occupancy detection with debouncing and state transitions

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "config.h"
#include "sensor.h"
#include "timing.h"
#include "data_models.h"

// ===== State Variables =====
bool isOccupied = false;
unsigned long sessionStartEpoch = 0;
int debounceCounter = 0;
static int carCounter = 0;

// ===== LED Control =====
void initLEDs() {
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    // Green = free, Red = occupied
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
}

void setLEDStatus(bool occupied) {
    if (occupied) {
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
    } else {
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
    }
}

// ===== Update Parking State =====
// Monitors distance and manages state transitions with debouncing
void updateParkingState(float distance) {
    if (distance < THRESHOLD_CM && !isOccupied) {
        // Potential car arrival - wait for confirmation
        debounceCounter++;
        
        if (debounceCounter >= DEBOUNCE_THRESHOLD) {
            // Car arrived (confirmed across multiple readings)
            isOccupied = true;
            sessionStartEpoch = getEpochNow();
            debounceCounter = 0;
            setLEDStatus(true);
            carCounter++;
            
            Serial.println("[PARKING] ✓ Car #" + String(carCounter) + " arrived");
            Serial.println("[PARKING]   Entry time: " + formatRealTime(sessionStartEpoch));
        }
    }
    else if (distance >= THRESHOLD_CM && isOccupied) {
        // Potential car departure - wait for confirmation
        debounceCounter++;
        
        if (debounceCounter >= DEBOUNCE_THRESHOLD) {
            // Car left (confirmed across multiple readings)
            isOccupied = false;
            unsigned long exitEpoch = getEpochNow();
            unsigned long stayDuration = exitEpoch - sessionStartEpoch;
            debounceCounter = 0;
            setLEDStatus(false);
            
            // Record session if it meets minimum duration
            if (stayDuration >= MIN_SESSION_SECONDS) {
                ParkingSession session = {
                    carCounter,
                    sessionStartEpoch,
                    exitEpoch,
                    stayDuration
                };
                addSessionToHistory(session);
                
                Serial.println("[PARKING] ✓ Car #" + String(carCounter) + " departed");
                Serial.println("[PARKING]   Exit time: " + formatRealTime(exitEpoch));
                Serial.println("[PARKING]   Duration: " + formatDuration(stayDuration));
            } else {
                Serial.println("[PARKING] ⊘ Car #" + String(carCounter) + " ignored (stayed " + String(stayDuration) + "s)");
            }
        }
    }
    else {
        // Distance threshold not crossed, reset debounce counter
        debounceCounter = 0;
    }
}

// ===== Get Current Status =====
bool getOccupancyStatus() {
    return isOccupied;
}

// ===== Get Total Visits =====
int getTotalVisits() {
    return carCounter;
}

#endif // STATE_MACHINE_H
