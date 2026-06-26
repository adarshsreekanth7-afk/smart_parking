#include <WiFi.h>
#include <WebServer.h>

// ===== Include all modular headers =====
#include "config.h"
#include "sensor.h"
#include "timing.h"
#include "data_models.h"
#include "state_machine.h"
#include "web_interface.h"

WebServer server(WEB_SERVER_PORT);
unsigned long lastSensorPoll = 0;

// ===== Setup =====
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n======================================");
    Serial.println("   Smart Parking System - Initializing");
    Serial.println("======================================\n");
    
    // Initialize hardware
    initSensor();
    initLEDs();
    
    Serial.println("[INIT] Hardware initialized");
    
    // WiFi setup (AP mode)
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASS);
    
    Serial.println("[WIFI] Access Point started");
    Serial.print("[WIFI] SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("[WIFI] IP: ");
    Serial.println(WiFi.softAPIP());
    
    // Attempt NTP sync (optional)
    Serial.println("\n[TIME] Attempting NTP sync...");
    bool ntpSuccess = syncNTP();
    if (!ntpSuccess) {
        Serial.println("[TIME] Using fallback epoch");
    }
    
    // Web server routes
    server.on("/", handleRoot);
    server.on("/api/status", handleAPI);
    server.on("/settime", handleSetTime);
    
    server.begin();
    Serial.println("\n[SERVER] Web server started on port " + String(WEB_SERVER_PORT));
    Serial.println("\n[READY] System ready. Navigate to http://192.168.4.1/ to view dashboard\n");
}

// ===== Main Loop =====
void loop() {
    // Handle web server requests
    server.handleClient();
    
    // Poll sensor at configured interval
    unsigned long now = millis();
    if (now - lastSensorPoll >= SENSOR_POLL_MS) {
        lastSensorPoll = now;
        
        // Get filtered distance and update state
        float distance = getDistanceFiltered();
        updateParkingState(distance);
    }
}

// ===== Web Server Handlers =====

void handleRoot() {
    server.send(200, "text/html", getHTML());
}

void handleAPI() {
    server.send(200, "application/json", getJSON());
}

void handleSetTime() {
    if (server.hasArg("epoch")) {
        unsigned long newEpoch = server.arg("epoch").toInt();
        setBootEpoch(newEpoch);
        Serial.println("[TIME] Manual epoch set: " + formatRealTime(getEpochNow()));
        server.send(200, "text/plain", "Time updated: " + formatRealTime(getEpochNow()));
    } else {
        server.send(400, "text/plain", "Missing ?epoch=<unix_timestamp> parameter");
    }
}
