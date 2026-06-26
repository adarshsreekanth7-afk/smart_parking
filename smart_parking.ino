#include <WiFi.h>
#include <WebServer.h>
#include <vector>
#include <time.h>

// ===== Configuration =====
#include "secrets.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

#define TRIG_PIN 5
#define ECHO_PIN 18
#define RED_LED 26
#define GREEN_LED 14

const float THRESHOLD_CM = 20.0;
const unsigned long MIN_SESSION_SECONDS = 10; // Minimum duration to record history

// ===== Time Tracking =====
// We use millis() as base and track a "boot epoch" offset
// Set this to the actual Unix timestamp of when the device boots
// For demo: we simulate a real time by adding an offset to millis()
// In production: sync via NTP or set via web interface
unsigned long bootEpoch = 1700000000UL; // fallback base epoch (Nov 2023)

String formatRealTime(unsigned long epochSec) {
    time_t t = (time_t)epochSec;
    struct tm* tmInfo = localtime(&t);
    char buf[30];
    // Format: DD/MM/YYYY HH:MM:SS
    strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", tmInfo);
    return String(buf);
}

unsigned long getEpochNow() {
    return bootEpoch + (millis() / 1000);
}

// ===== Data Structures =====
struct ParkingSession {
    int carID;
    unsigned long entryEpoch;   // real unix timestamp
    unsigned long exitEpoch;    // real unix timestamp
    unsigned long duration;     // in seconds
};

std::vector<ParkingSession> history;
int carCounter = 0;
bool isOccupied = false;
unsigned long sessionStartEpoch = 0;
unsigned long sessionStartMillis = 0;
float currentDistance = 0;

WebServer server(80);

// ===== Distance Sensing =====
float getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long dur = pulseIn(ECHO_PIN, HIGH, 30000);
    if (dur == 0) return 400.0;
    return dur * 0.034 / 2;
}

// ===== Duration Formatter =====
String formatDuration(unsigned long secs) {
    if (secs < 60) return String(secs) + " sec";
    unsigned long mins = secs / 60;
    unsigned long rem = secs % 60;
    if (mins < 60) return String(mins) + "m " + String(rem) + "s";
    unsigned long hrs = mins / 60;
    mins = mins % 60;
    return String(hrs) + "h " + String(mins) + "m " + String(rem) + "s";
}

// ===== Web Page =====
String getHTML() {
    String ptr = R"rawhtml(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta http-equiv="refresh" content="3">
<title>Smart Parking — Slot 101</title>
<link href="https://fonts.googleapis.com/css2?family=DM+Mono:wght@400;500&family=Syne:wght@700;800&family=Inter:wght@400;500&display=swap" rel="stylesheet">
<style>
  :root {
    --bg: #0a0c10;
    --surface: #111318;
    --surface2: #181b22;
    --border: #1e2230;
    --accent-green: #00e87a;
    --accent-red: #ff3b5c;
    --accent-blue: #3b82f6;
    --text: #e8eaf0;
    --text-dim: #6b7280;
    --text-mid: #9ca3af;
    --glow-green: 0 0 24px rgba(0,232,122,0.25);
    --glow-red: 0 0 24px rgba(255,59,92,0.25);
  }

  * { box-sizing: border-box; margin: 0; padding: 0; }

  body {
    font-family: 'Inter', sans-serif;
    background: var(--bg);
    color: var(--text);
    min-height: 100vh;
    padding: 24px 16px 60px;
  }

  /* Background grid */
  body::before {
    content: '';
    position: fixed;
    inset: 0;
    background-image:
      linear-gradient(rgba(59,130,246,0.03) 1px, transparent 1px),
      linear-gradient(90deg, rgba(59,130,246,0.03) 1px, transparent 1px);
    background-size: 40px 40px;
    pointer-events: none;
    z-index: 0;
  }

  .wrap {
    max-width: 760px;
    margin: 0 auto;
    position: relative;
    z-index: 1;
  }

  /* ---- HEADER ---- */
  .header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-bottom: 32px;
    padding-bottom: 20px;
    border-bottom: 1px solid var(--border);
  }
  .header-left h1 {
    font-family: 'Syne', sans-serif;
    font-size: 1.6rem;
    font-weight: 800;
    letter-spacing: -0.5px;
    color: var(--text);
  }
  .header-left p {
    font-family: 'DM Mono', monospace;
    font-size: 0.72rem;
    color: var(--text-dim);
    margin-top: 4px;
    letter-spacing: 0.08em;
    text-transform: uppercase;
  }
  .live-dot {
    display: flex;
    align-items: center;
    gap: 8px;
    font-family: 'DM Mono', monospace;
    font-size: 0.72rem;
    color: var(--text-dim);
    text-transform: uppercase;
    letter-spacing: 0.1em;
  }
  .live-dot::before {
    content: '';
    width: 8px; height: 8px;
    border-radius: 50%;
    background: var(--accent-green);
    box-shadow: 0 0 8px var(--accent-green);
    animation: pulse 1.5s infinite;
  }
  @keyframes pulse {
    0%,100% { opacity: 1; }
    50% { opacity: 0.3; }
  }

  /* ---- STATUS CARD ---- */
  .status-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 20px;
    padding: 36px 32px;
    margin-bottom: 28px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 24px;
    position: relative;
    overflow: hidden;
    transition: box-shadow 0.4s ease;
  }
  .status-card.occupied {
    border-color: rgba(255,59,92,0.3);
    box-shadow: var(--glow-red);
  }
  .status-card.available {
    border-color: rgba(0,232,122,0.3);
    box-shadow: var(--glow-green);
  }
  .status-card::after {
    content: '';
    position: absolute;
    top: -40px; right: -40px;
    width: 140px; height: 140px;
    border-radius: 50%;
    opacity: 0.06;
  }
  .status-card.occupied::after { background: var(--accent-red); }
  .status-card.available::after { background: var(--accent-green); }

  .status-left {}
  .slot-label {
    font-family: 'DM Mono', monospace;
    font-size: 0.7rem;
    color: var(--text-dim);
    text-transform: uppercase;
    letter-spacing: 0.12em;
    margin-bottom: 12px;
  }
  .status-badge {
    display: inline-flex;
    align-items: center;
    gap: 10px;
    padding: 10px 22px;
    border-radius: 100px;
    font-family: 'Syne', sans-serif;
    font-weight: 700;
    font-size: 1rem;
    letter-spacing: 0.02em;
  }
  .status-badge.occupied {
    background: rgba(255,59,92,0.12);
    color: var(--accent-red);
    border: 1.5px solid rgba(255,59,92,0.35);
  }
  .status-badge.available {
    background: rgba(0,232,122,0.1);
    color: var(--accent-green);
    border: 1.5px solid rgba(0,232,122,0.3);
  }
  .status-badge .dot {
    width: 9px; height: 9px;
    border-radius: 50%;
  }
  .status-badge.occupied .dot { background: var(--accent-red); box-shadow: 0 0 6px var(--accent-red); }
  .status-badge.available .dot { background: var(--accent-green); box-shadow: 0 0 6px var(--accent-green); animation: pulse 1.5s infinite; }

  .status-sub {
    font-size: 0.82rem;
    color: var(--text-dim);
    margin-top: 12px;
  }

  .status-right {
    text-align: right;
  }
  .car-count-label {
    font-family: 'DM Mono', monospace;
    font-size: 0.68rem;
    color: var(--text-dim);
    text-transform: uppercase;
    letter-spacing: 0.1em;
  }
  .car-count-num {
    font-family: 'Syne', sans-serif;
    font-size: 3rem;
    font-weight: 800;
    color: var(--text);
    line-height: 1;
    margin-top: 4px;
  }

  /* ---- SESSION IN PROGRESS ---- */
  .session-bar {
    background: var(--surface);
    border: 1px solid rgba(255,59,92,0.2);
    border-left: 3px solid var(--accent-red);
    border-radius: 12px;
    padding: 14px 20px;
    margin-bottom: 28px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 12px;
  }
  .session-bar-label {
    font-family: 'DM Mono', monospace;
    font-size: 0.72rem;
    color: var(--accent-red);
    text-transform: uppercase;
    letter-spacing: 0.1em;
  }
  .session-bar-time {
    font-family: 'DM Mono', monospace;
    font-size: 0.85rem;
    color: var(--text-mid);
  }

  /* ---- SECTION HEADING ---- */
  .section-head {
    display: flex;
    align-items: baseline;
    gap: 10px;
    margin-bottom: 16px;
  }
  .section-head h2 {
    font-family: 'Syne', sans-serif;
    font-size: 1.1rem;
    font-weight: 700;
    color: var(--text);
  }
  .section-head .count-pill {
    font-family: 'DM Mono', monospace;
    font-size: 0.68rem;
    background: var(--surface2);
    border: 1px solid var(--border);
    color: var(--text-dim);
    padding: 2px 10px;
    border-radius: 100px;
    letter-spacing: 0.05em;
  }

  /* ---- TABLE ---- */
  .table-wrap {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 16px;
    overflow: hidden;
  }
  table {
    width: 100%;
    border-collapse: collapse;
  }
  thead th {
    background: var(--surface2);
    padding: 13px 18px;
    text-align: left;
    font-family: 'DM Mono', monospace;
    font-size: 0.68rem;
    color: var(--text-dim);
    text-transform: uppercase;
    letter-spacing: 0.1em;
    border-bottom: 1px solid var(--border);
    font-weight: 500;
  }
  tbody td {
    padding: 14px 18px;
    font-size: 0.85rem;
    border-bottom: 1px solid var(--border);
    color: var(--text-mid);
    vertical-align: middle;
  }
  tbody tr:last-child td { border-bottom: none; }
  tbody tr:hover td { background: var(--surface2); }

  .car-id {
    font-family: 'DM Mono', monospace;
    font-size: 0.8rem;
    color: var(--accent-blue);
    background: rgba(59,130,246,0.1);
    border: 1px solid rgba(59,130,246,0.2);
    padding: 3px 10px;
    border-radius: 6px;
    display: inline-block;
  }
  .time-val {
    font-family: 'DM Mono', monospace;
    font-size: 0.78rem;
    color: var(--text-mid);
  }
  .dur-val {
    font-family: 'DM Mono', monospace;
    font-size: 0.8rem;
    color: var(--text);
    background: var(--surface2);
    border: 1px solid var(--border);
    padding: 3px 10px;
    border-radius: 6px;
    display: inline-block;
  }

  .empty-row td {
    text-align: center;
    color: var(--text-dim);
    font-size: 0.85rem;
    padding: 40px 18px;
    font-style: italic;
  }

  /* ---- FOOTER ---- */
  .footer {
    text-align: center;
    margin-top: 40px;
    font-family: 'DM Mono', monospace;
    font-size: 0.65rem;
    color: var(--text-dim);
    letter-spacing: 0.08em;
    text-transform: uppercase;
  }

  @media(max-width: 520px) {
    .status-card { flex-direction: column; align-items: flex-start; }
    .status-right { text-align: left; }
    .header { flex-direction: column; align-items: flex-start; gap: 12px; }
  }
</style>
</head>
<body>
<div class="wrap">

  <div class="header">
    <div class="header-left">
      <h1>&#x1F17F; Smart Parking</h1>
      <p>Slot #101 &nbsp;&bull;&nbsp; Live Monitor</p>
    </div>
    <div class="live-dot">Auto-refresh 3s</div>
  </div>

  <div class="status-card )rawhtml";

    ptr += isOccupied ? "occupied" : "available";
    ptr += R"rawhtml(">
    <div class="status-left">
      <div class="slot-label">Current Status</div>
      <div class="status-badge )rawhtml";
    ptr += isOccupied ? "occupied" : "available";
    ptr += R"rawhtml(">
        <span class="dot"></span>
        )rawhtml";
    ptr += isOccupied ? "OCCUPIED" : "AVAILABLE";
    ptr += R"rawhtml(
      </div>
      <div class="status-sub">)rawhtml";
    ptr += isOccupied ? "&#x1F697; Vehicle detected in slot" : "&#x2705; Slot is free to park";
    ptr += R"rawhtml(</div>
    </div>
    <div class="status-right">
      <div class="car-count-label">Total Visits</div>
      <div class="car-count-num">)rawhtml";
    ptr += String(carCounter);
    ptr += R"rawhtml(</div>
    </div>
  </div>
)rawhtml";

    // Show active session bar if occupied
    if (isOccupied) {
        unsigned long elapsed = getEpochNow() - sessionStartEpoch;
        ptr += "<div class='session-bar'>";
        ptr += "<div class='session-bar-label'>&#x23F1; Session in progress</div>";
        ptr += "<div class='session-bar-time'>Started: " + formatRealTime(sessionStartEpoch) + " &nbsp;|&nbsp; Elapsed: " + formatDuration(elapsed) + "</div>";
        ptr += "</div>";
    }

    // History Table
    int recordCount = history.size();
    ptr += "<div class='section-head'><h2>Parking History</h2><span class='count-pill'>" + String(recordCount) + " records</span></div>";
    ptr += "<div class='table-wrap'><table><thead><tr>";
    ptr += "<th>Car</th><th>Entry Time</th><th>Exit Time</th><th>Duration</th>";
    ptr += "</tr></thead><tbody>";

    if (history.empty()) {
        ptr += "<tr class='empty-row'><td colspan='4'>No parking records yet &mdash; sessions under 10 seconds are not logged</td></tr>";
    } else {
        int count = 0;
        for (int i = history.size() - 1; i >= 0 && count < 15; i--) {
            ptr += "<tr>";
            ptr += "<td><span class='car-id'>#" + String(history[i].carID) + "</span></td>";
            ptr += "<td><span class='time-val'>" + formatRealTime(history[i].entryEpoch) + "</span></td>";
            ptr += "<td><span class='time-val'>" + formatRealTime(history[i].exitEpoch) + "</span></td>";
            ptr += "<td><span class='dur-val'>" + formatDuration(history[i].duration) + "</span></td>";
            ptr += "</tr>";
            count++;
        }
    }

    ptr += "</tbody></table></div>";
    ptr += "<div class='footer'>Smart Parking System &nbsp;&bull;&nbsp; ESP32 &nbsp;&bull;&nbsp; Only sessions &ge;10s are recorded</div>";
    ptr += "</div></body></html>";
    return ptr;
}

void setup() {
    Serial.begin(115200);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);

    digitalWrite(GREEN_LED, HIGH);

    // ---- NTP Time Sync (optional, requires internet) ----
    // If you have internet access via a router, uncomment:
    // configTime(19800, 0, "pool.ntp.org"); // IST = UTC+5:30 = 19800s
    // After sync, bootEpoch is set automatically via time()
    // For AP-only mode, manually set bootEpoch to current Unix timestamp:
    // bootEpoch = 1700000000UL; // <-- Replace with current Unix time at boot

    WiFi.softAP(ssid, password);
    Serial.println("Access Point Started");
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", []() {
        server.send(200, "text/html", getHTML());
    });

    // Optional: endpoint to set real time via GET param ?epoch=XXXXXXXXXX
    server.on("/settime", []() {
        if (server.hasArg("epoch")) {
            bootEpoch = server.arg("epoch").toInt() - (millis() / 1000);
            server.send(200, "text/plain", "Time set OK");
        } else {
            server.send(400, "text/plain", "Missing ?epoch=");
        }
    });

    server.begin();
}

void loop() {
    server.handleClient();

    currentDistance = getDistance();

    if (currentDistance < THRESHOLD_CM && !isOccupied) {
        // Car arrived
        isOccupied = true;
        carCounter++;
        sessionStartEpoch = getEpochNow();
        sessionStartMillis = millis();
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
        Serial.println("Car #" + String(carCounter) + " arrived at " + formatRealTime(sessionStartEpoch));
    }
    else if (currentDistance >= THRESHOLD_CM && isOccupied) {
        // Car left
        isOccupied = false;
        unsigned long exitEpoch = getEpochNow();
        unsigned long stayDuration = exitEpoch - sessionStartEpoch;

        // Only record if stayed at least 10 seconds
        if (stayDuration >= MIN_SESSION_SECONDS) {
            ParkingSession session;
            session.carID = carCounter;
            session.entryEpoch = sessionStartEpoch;
            session.exitEpoch = exitEpoch;
            session.duration = stayDuration;
            history.push_back(session);
            Serial.println("Car #" + String(carCounter) + " recorded. Duration: " + String(stayDuration) + "s");
        } else {
            carCounter--; // Don't count this as a real visit
            Serial.println("Car ignored (stayed < 10s): " + String(stayDuration) + "s");
        }

        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
    }

    delay(500);
}
