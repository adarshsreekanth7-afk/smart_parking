# 🅿️ Smart Parking System

An ESP32-based smart parking slot monitor with a built-in web dashboard, real-time occupancy detection, and session history logging — accessible over your local WiFi network.

---

## Features

- **Real-time detection** — HC-SR04 ultrasonic sensor detects vehicle presence within a configurable threshold
- **Web dashboard** — Live status page served directly from the ESP32, no app needed
- **Session history** — Logs parking sessions with timestamps (start/end) filtered by minimum duration
- **Visual indicators** — RGB LED shows slot status at a glance (green = free, red = occupied)
- **Time tracking** — Boot-epoch based timestamps, NTP-syncable for production use

---

## Hardware

| Component | Pin |
|-----------|-----|
| ESP32 Dev Board | — |
| HC-SR04 Ultrasonic Sensor | TRIG → 5, ECHO → 18 |
| Red LED | GPIO 26 |
| Green LED | GPIO 14 |

**Detection threshold:** 20 cm (configurable via `THRESHOLD_CM`)

---

## Getting Started

### 1. Clone the repo

```bash
git clone https://github.com/adarshsreekanth7-afk/smart_parking.git
cd smart_parking
```

### 2. Create your secrets file

Copy the template and fill in your credentials:

```bash
cp firmware/smart_parking/secrets_template.h firmware/smart_parking/secrets.h
```

Edit `secrets.h`:

```cpp
#define WIFI_SSID "your_network_name"
#define WIFI_PASS "your_password"
```

> `secrets.h` is listed in `.gitignore` — it will never be committed.

### 3. Flash the firmware

Open `firmware/smart_parking/smart_parking.ino` in Arduino IDE, select your ESP32 board, and upload.

### 4. Access the dashboard

Open the Serial Monitor at 115200 baud to find the ESP32's IP address, then navigate to it in any browser on your network:

```
http://192.168.x.x/
```

---

## Configuration

All tuneable parameters are at the top of `smart_parking.ino`:

```cpp
const float THRESHOLD_CM = 20.0;          // Distance to trigger "occupied"
const unsigned long MIN_SESSION_SECONDS = 10; // Ignore sessions shorter than this
```

For real timestamps, replace `bootEpoch` with an NTP sync call (see `docs/ntp_setup.md`).

---

## Project Structure

```
smart_parking/
├── firmware/
│   └── smart_parking/
│       ├── smart_parking.ino
│       └── secrets.h          ← you create this (gitignored)
├── hardware/
│   ├── schematic.pdf
│   └── BOM.csv
├── docs/
│   └── ntp_setup.md
├── .gitignore
├── LICENSE
└── README.md
```

---

## Roadmap

- [ ] NTP time sync on boot
- [ ] Multiple slot support
- [ ] Push notifications on status change
- [ ] OLED display support
- [ ] MQTT integration for home automation

---

## License

MIT License — see [LICENSE](LICENSE) for details.
