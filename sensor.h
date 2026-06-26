// =====================================================
// sensor.h - HC-SR04 Ultrasonic Sensor Utilities
// =====================================================
// Handles distance measurement with filtering and error handling

#ifndef SENSOR_H
#define SENSOR_H

#include "config.h"

// ===== Sensor State =====
static float sensorReadings[SENSOR_READINGS] = {0};
static int readingIndex = 0;

// ===== Initialize Sensor Pins =====
void initSensor() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

// ===== Raw Distance Measurement =====
// Returns distance in centimeters, or 400cm if measurement fails
float getDistanceRaw() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long dur = pulseIn(ECHO_PIN, HIGH, 30000);
    if (dur == 0) return 400.0;
    
    // Speed of sound: 0.034 cm/microsecond, divide by 2 for round trip
    return dur * 0.034 / 2;
}

// ===== Filtered Distance with Moving Average =====
// Reduces sensor noise and spurious readings
float getDistanceFiltered() {
    // Add new reading to circular buffer
    sensorReadings[readingIndex] = getDistanceRaw();
    readingIndex = (readingIndex + 1) % SENSOR_READINGS;
    
    // Calculate moving average
    float sum = 0;
    for (int i = 0; i < SENSOR_READINGS; i++) {
        sum += sensorReadings[i];
    }
    
    return sum / SENSOR_READINGS;
}

#endif // SENSOR_H
