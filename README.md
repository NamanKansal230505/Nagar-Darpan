<div align="center">

# Nagar-Darpan

### _The City Mirror — See Your City. Shape Your City._

A gamified, IoT-powered civic engagement platform that monitors and improves urban neighborhoods through real-time environmental sensors, autonomous drone inspections, and a citizen reward system.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)]()
[![Platform](https://img.shields.io/badge/platform-Web%20%7C%20IoT-brightgreen.svg)]()
[![Firebase](https://img.shields.io/badge/backend-Firebase-orange.svg)]()
[![ESP32](https://img.shields.io/badge/hardware-ESP32-red.svg)]()

</div>

---

## Table of Contents

- [Problem Statement](#problem-statement)
- [Our Solution](#our-solution)
- [Key Features](#key-features)
- [Tech Stack](#tech-stack)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Getting Started](#getting-started)
- [Hardware Setup](#hardware-setup)
- [Deployment](#deployment)
- [Firebase Configuration](#firebase-configuration)
- [API Reference](#api-reference)
- [Screenshots](#screenshots)
- [Team](#team)

---

## Problem Statement

Indian cities face a growing crisis in urban livability. Air pollution, inadequate waste management, deteriorating water quality, and electricity theft are problems that persist — not because solutions don't exist, but because **cities lack real-time visibility** into these issues at a granular, ward-level scale.

Current civic systems suffer from:

- **No real-time monitoring** — Environmental data is collected manually and infrequently, leaving authorities blind to emerging hazards.
- **Zero citizen engagement** — Residents have no accessible way to see how their neighborhood performs or to contribute to its improvement.
- **Siloed data** — Air quality, water quality, waste, and electricity metrics are tracked by different departments with no unified view.
- **Reactive governance** — Municipal bodies act only after complaints, rather than proactively preventing problems.

---

## Our Solution

**Nagar-Darpan** (City Mirror) provides a unified, real-time, data-driven platform that connects IoT sensors deployed across city wards to a centralized dashboard — making environmental health visible, comparable, and actionable.

The platform introduces three core innovations:

1. **MONITOR** — IoT sensors (air quality, water quality, waste bin levels) stream data 24/7 to Firebase, providing continuous ward-level environmental metrics.
2. **ANALYZE & RANK** — Wards are scored, ranked on a live leaderboard, and color-coded (Good / Moderate / Poor) so municipal officials can prioritize interventions instantly.
3. **REWARD** — Citizens earn **CivicPoints** when their ward improves, redeemable at local businesses, public transit, and community projects — turning civic responsibility into a tangible incentive.

---

## Key Features

| Feature | Description | Status |
|---|---|---|
| **Real-Time AQI Monitoring** | MQ2 gas sensors measure PM2.5, PM10, O3, CO, LPG, and more — pushed live to the dashboard | Active |
| **Water Quality Tracking** | TDS sensors measure Total Dissolved Solids in ppm for real-time water safety scoring | Active |
| **Smart Waste Management** | Ultrasonic sensors track dustbin fill percentage and alert when bins are nearing capacity | Active |
| **Autonomous Drone Inspection** | ESP32-CAM streams live video; Roboflow computer vision detects garbage and violations | Active |
| **Electricity Theft Detection** | Real-time anomaly detection on power lines with visual alert system | Active |
| **Ward Leaderboard** | Live ranking of wards across AQI, Cleanliness, and Water Quality with trophy indicators | Active |
| **CivicPoints Gamification** | Citizens earn and redeem points based on ward performance improvements | Active |
| **Multi-Role Authentication** | Separate portals for Municipal Admins, Ward Officers, and Citizens | Active |
| **Interactive City Map** | Leaflet-powered ward visualization with zone overlays and color-coded status | Active |
| **Responsive Glass-Morphism UI** | Modern, accessible interface with animated mesh visualizations and scroll-driven effects | Active |

---

## Tech Stack

### Frontend

| Technology | Purpose |
|---|---|
| **HTML5 / CSS3 / JavaScript (ES6+)** | Core web technologies — no framework overhead |
| **Leaflet 1.9.4** | Interactive map rendering for ward visualization |
| **Canvas API** | Animated mesh network hero visualization (Prim's MST algorithm) |
| **Google Fonts (Outfit)** | Typography with variable weight (300–900) |
| **CSS Glass-Morphism** | Backdrop filters, gradient borders, floating orbs |

### Backend & Cloud

| Technology | Purpose |
|---|---|
| **Firebase Realtime Database** | Sub-second data sync between IoT sensors and web dashboards |
| **Firebase Authentication** | Email/password auth with multi-role access control |
| **Python HTTP Server** | Lightweight local development and LAN serving |

### Hardware & IoT

| Component | Purpose |
|---|---|
| **ESP32** | Primary microcontroller for all sensor nodes |
| **ESP32-CAM** | MJPEG video streaming for drone inspection |
| **MQ2 Gas Sensor** | Air quality measurement (10 gas types) |
| **TDS Sensor** | Water quality measurement (Total Dissolved Solids) |
| **HC-SR04 Ultrasonic Sensor** | Dustbin fill-level detection |
| **Firebase ESP Client Library** | Secure IoT-to-cloud data pipeline |

### External APIs

| API | Purpose |
|---|---|
| **Roboflow** | Computer vision model for garbage/violation detection |
| **Firebase REST API** | Real-time data read/write from sensor nodes |

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                         NAGAR-DARPAN ARCHITECTURE                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐           │
│  │  MQ2 Sensor  │   │  TDS Sensor  │   │  HC-SR04     │           │
│  │  (Air Quality)│   │ (Water Quality│   │  (Waste Bin) │           │
│  └──────┬───────┘   └──────┬───────┘   └──────┬───────┘           │
│         │                  │                   │                    │
│         └──────────────────┼───────────────────┘                   │
│                            │                                        │
│                    ┌───────▼────────┐                               │
│                    │     ESP32      │                               │
│                    │  (WiFi + ADC)  │                               │
│                    └───────┬────────┘                               │
│                            │  HTTPS                                 │
│                    ┌───────▼────────┐    ┌──────────────┐          │
│                    │    Firebase    │◄───│  ESP32-CAM   │          │
│                    │  Realtime DB   │    │ (Video Stream)│          │
│                    │  + Auth        │    └──────────────┘          │
│                    └───────┬────────┘                               │
│                            │  WebSocket                             │
│              ┌─────────────┼─────────────┐                         │
│              │             │             │                          │
│      ┌───────▼──────┐ ┌───▼──────┐ ┌───▼──────────┐              │
│      │  Municipal   │ │  Ward    │ │   Citizen    │              │
│      │  Dashboard   │ │ Dashboard│ │   Portal     │              │
│      └──────────────┘ └──────────┘ └──────────────┘              │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Project Structure

```
Nagar-Darpan/
├── index.html                        # Landing page with hero, features, and CTA
├── municipal-login.html              # Municipal / Ward admin login portal
├── municipal-dashboard.html          # City-wide monitoring dashboard
├── citizen-login-page.html           # Citizen login portal
├── citizen-login.html                # Citizen authentication page
├── electricity-theft.html            # Electricity theft detection module
├── inspection.html                   # Drone inspection with live video feed
│
├── css/
│   └── styles.css                    # Global styles — glass-morphism, animations, responsive grid
│
├── js/
│   └── script.js                     # Client logic — canvas mesh, modals, observers, navigation
│
├── assets/                           # Static media
│   ├── *.svg                         # Feature icons (AQI, drone, map, transit, etc.)
│   ├── drone.gif                     # Animated drone visualization
│   └── siren-alert-96052.mp3        # Alert audio for theft detection
│
├── hardware/                         # ESP32 Arduino firmware
│   ├── ESP32_CAM_Stream.ino          # MJPEG video stream from ESP32-CAM
│   ├── ESP32_MQ2_Firebase.ino        # Air quality sensor → Firebase
│   ├── ESP32_TDS_Firebase.ino        # Water quality sensor → Firebase
│   ├── ESP32_Dustbin_Firebase.ino    # Waste bin level sensor → Firebase
│   └── MQ2_AQI_Sensor.ino           # Standalone AQI calculator
│
├── start-server.sh                   # Linux/macOS server startup
├── start-server.bat                  # Windows server startup
└── README.md                         # You are here
```

---

## Prerequisites

| Requirement | Version | Purpose |
|---|---|---|
| **Python** | 3.x | Local HTTP server |
| **Modern Browser** | Latest | Chrome, Firefox, Safari, or Edge |
| **Internet Connection** | — | Firebase, Google Fonts, Leaflet CDN |
| **Arduino IDE** | 2.x+ | Firmware upload to ESP32 (hardware only) |
| **ESP32 Board Package** | Latest | Arduino board manager (hardware only) |

### Arduino Libraries (Hardware Setup Only)

- `Firebase_ESP_Client`
- `WiFi` (built-in with ESP32 board package)
- `esp_camera` (built-in with ESP32-CAM board package)

---

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/NamanKansal230505/Nagar-Darpan.git
cd Nagar-Darpan
```

### 2. Start the Development Server

**Windows:**
```bash
start-server.bat
```

**Linux / macOS:**
```bash
chmod +x start-server.sh
./start-server.sh
```

**Manual:**
```bash
python3 -m http.server 8000
```

### 3. Open in Browser

| Page | URL |
|---|---|
| Landing Page | `http://localhost:8000` |
| Municipal Dashboard | `http://localhost:8000/municipal-dashboard.html` |
| Citizen Portal | `http://localhost:8000/citizen-login-page.html` |
| Drone Inspection | `http://localhost:8000/inspection.html` |
| Electricity Theft | `http://localhost:8000/electricity-theft.html` |

### 4. Demo Credentials

| Role | Email | Password |
|---|---|---|
| Citizen | `naman.saarthi@gmail.com` | `12345678` |

---

## Hardware Setup

### Wiring Diagrams

#### MQ2 Air Quality Sensor
| MQ2 Pin | ESP32 Pin |
|---|---|
| VCC | 3.3V |
| GND | GND |
| AOUT | GPIO 34 (ADC1_CH6) |

#### TDS Water Quality Sensor
| TDS Pin | ESP32 Pin |
|---|---|
| VCC | 3.3V |
| GND | GND |
| AOUT | GPIO 34 |

#### HC-SR04 Ultrasonic (Dustbin)
| HC-SR04 Pin | ESP32 Pin |
|---|---|
| VCC | 5V |
| GND | GND |
| TRIG | GPIO 5 |
| ECHO | GPIO 18 |

### Flashing Firmware

1. Open the desired `.ino` file from the `hardware/` directory in Arduino IDE.
2. Select your ESP32 board under **Tools > Board > ESP32 Dev Module** (or **AI Thinker ESP32-CAM** for the camera module).
3. Update the WiFi credentials in the sketch:
   ```cpp
   #define WIFI_SSID     "YOUR_WIFI_SSID"
   #define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"
   ```
4. Update Firebase credentials if using your own project:
   ```cpp
   #define API_KEY        "YOUR_FIREBASE_API_KEY"
   #define DATABASE_URL   "YOUR_FIREBASE_DATABASE_URL"
   ```
5. Upload the sketch. Monitor output via **Tools > Serial Monitor** at `115200` baud.
6. Once connected, sensor data will begin streaming to Firebase automatically.

---

## Deployment

### Option A — Static Hosting (Frontend Only)

The frontend is fully static and can be deployed to any hosting provider:

**Firebase Hosting:**
```bash
npm install -g firebase-tools
firebase login
firebase init hosting    # Set public directory to "."
firebase deploy
```

**Netlify / Vercel:**
1. Push the repository to GitHub.
2. Connect the repo in the Netlify or Vercel dashboard.
3. Set the build command to _empty_ and publish directory to `.` (root).
4. Deploy.

**GitHub Pages:**
1. Go to **Settings > Pages** in your GitHub repository.
2. Set source to the `main` branch, root directory.
3. Save — your site will be live at `https://<username>.github.io/Nagar-Darpan/`.

### Option B — LAN Deployment (With Hardware)

For a complete deployment with IoT sensors on a local network:

1. Deploy the frontend on a machine connected to the same WiFi as the ESP32 sensors.
2. Start the Python server:
   ```bash
   python3 -m http.server 8000 --bind 0.0.0.0
   ```
3. Access the dashboard from any device on the network via `http://<HOST_IP>:8000`.
4. Ensure all ESP32 modules are powered on and connected to the same WiFi network.
5. Verify data flow: **Sensor → Firebase → Dashboard** (check the Firebase Console for incoming writes).

---

## Firebase Configuration

### Database Structure

```
saarthi-84622-default-rtdb/
│
├── /aqi                          # Latest AQI reading from MQ2 sensor
│
├── /tds                          # Latest TDS reading (ppm)
│
├── /dustbin                      # Dustbin fill percentage (0–100%)
│
├── /wards/
│   └── {wardId}/
│       ├── name                  # Ward display name
│       ├── aqi                   # Current AQI value
│       ├── cleanliness           # Cleanliness score (0–100)
│       ├── tdsValue              # Water quality in ppm
│       ├── score                 # Composite environmental score
│       └── status                # "good" | "moderate" | "poor"
│
└── /users/
    └── {userId}/
        ├── email                 # Registered email
        ├── ward                  # Assigned ward
        ├── civicPoints           # Accumulated reward points
        └── profile               # Additional profile metadata
```

### Status Thresholds

| Metric | Good | Moderate | Poor |
|---|---|---|---|
| **AQI** | 0 – 50 | 51 – 100 | > 100 |
| **Cleanliness** | 80 – 100% | 50 – 79% | < 50% |
| **TDS** | 0 – 300 ppm | 301 – 600 ppm | > 600 ppm |

---

## API Reference

### Firebase Realtime Database Paths

| Method | Path | Description |
|---|---|---|
| `SET` | `/aqi` | Write latest AQI reading from sensor |
| `SET` | `/tds` | Write latest TDS reading from sensor |
| `SET` | `/dustbin` | Write dustbin fill percentage |
| `GET` | `/wards/{wardId}` | Fetch all metrics for a specific ward |
| `GET` | `/wards` | Fetch all wards with scores and rankings |
| `GET` | `/users/{userId}` | Fetch citizen profile and CivicPoints |

### External Integrations

| Service | Endpoint | Usage |
|---|---|---|
| **Roboflow** | Inference API | Garbage detection from drone camera feed |
| **Leaflet Tile Server** | OpenStreetMap tiles | Base map layer for dashboard visualization |

---

## Design System

| Token | Value | Usage |
|---|---|---|
| `--bg` | `#0b1220` | Primary background (deep midnight) |
| `--accent` | `#18e0b5` | Primary accent (electric teal) |
| `--accent-2` | `#43ff88` | Secondary accent (eco green) |
| `--good` | `#43ff88` | Good status indicator |
| `--moderate` | `#ffc857` | Moderate status indicator |
| `--poor` | `#ff6b6b` | Poor / critical status indicator |
| `--text` | `#dbe6ff` | Primary text color |
| `--glass-blur` | `16px` | Glass-morphism backdrop blur |
| `--radius` | `14px` | Default border radius |

---

## Contributing

1. **Fork** the repository.
2. **Create** a feature branch: `git checkout -b feature/your-feature`.
3. **Commit** your changes: `git commit -m "Add your feature"`.
4. **Push** to the branch: `git push origin feature/your-feature`.
5. **Open** a Pull Request against `main`.

Please ensure your code follows the existing style conventions and includes relevant documentation updates.

---

## License

This project is built for [Innotech 2025]([https://innotech.com](https://www.linkedin.com/feed/update/urn:li:activity:7396918764568502272/)https://www.linkedin.com/feed/update/urn:li:activity:7396918764568502272/). All rights reserved.

---

<div align="center">

**Built with purpose. Powered by data. Driven by citizens.**

_Nagar-Darpan — Because every city deserves a mirror._

</div>
