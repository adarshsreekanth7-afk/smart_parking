// =====================================================
// sensor.h - HC-SR04 Ultrasonic Sensor Utilities
// =====================================================
// Handles distance measurement and signal processing
// from the HC-SR04 ultrasonic sensor.

#ifndef SENSOR_H
#define SENSOR_H

#include "config.h"

// ===== Distance Sensing =====
// Measures distance using HC-SR04 ultrasonic sensor
// Returns distance in centimeters, or 400cm if measurement fails
float getDistance() {
    // Send trigger pulse
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    // Measure echo pulse duration
    long dur = pulseIn(ECHO_PIN, HIGH, 30000);
    
    // Return max distance if no echo received
    if (dur == 0) return 400.0;
    
    // Convert pulse duration to distance (cm)
    // Speed of sound: 0.034 cm/microsecond, divide by 2 (round trip)
    return dur * 0.034 / 2;
}

// ===== Sensor Initialization =====
// Call this in setup() to configure sensor pins
void initSensor() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

#endif // SENSOR_H
