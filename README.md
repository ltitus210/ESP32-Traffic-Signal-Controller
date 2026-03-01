# ESP32 Traffic Signal Controller
A fully configurable WiFi-enabled traffic light controller built for the ESP32 using the Arduino IDE. Currently it will control a single 3 module Traffic Head with a standard red, yellow, and green light configuration.

![gif](https://upload.wikimedia.org/wikipedia/commons/thumb/e/e8/Traffic_light_%28animation%29.gif/330px-Traffic_light_%28animation%29.gif)

This project supports:

* 🇺🇸 United States signal pattern
* 🇩🇪 German signal pattern (Red + Yellow before Green)
* 🎛 Custom manual control mode
* 🌐 Built-in web interface
* 💾 Persistent settings (timings, pattern, WiFi, custom states)
* 📡 WiFi STA + fallback Setup Access Point

---

# Features

## 1. Signal Patterns

### United States Pattern

```
GREEN → YELLOW → RED → GREEN
```

With a short all-red clearance between transitions.

---

### German Pattern

```
RED → RED+YELLOW → GREEN → YELLOW → RED
```

Includes a 1-second Red+Yellow pre-green state.

---

### Custom Pattern (Manual Mode)

* Timings are ignored.
* Webpage shows toggle buttons.
* User manually controls:

  * Red
  * Yellow
  * Green
* States persist across reboot.

---

# Web Interface

The ESP32 hosts a small HTTP server.

### Main Page (`/`)

* Pattern selector (US / Germany / Custom)
* Sliders for:

  * Green (1–60 sec)
  * Yellow (1–10 sec)
  * Red (1–60 sec)
* Custom toggle controls (only visible in Custom mode)
* Live status display
* Settings apply instantly

---

### WiFi Settings Page (`/settings`)

* Set SSID
* Set password
* Clear WiFi credentials
* Reboot device

If WiFi credentials are cleared or invalid:

* ESP32 starts Setup Access Point mode.

---

# Persistence (Flash Storage)

This project uses the ESP32 `Preferences` library (NVS storage).

The following settings are saved in flash:

| Setting            | Stored? |
| ------------------ | ------- |
| WiFi SSID          | ✅       |
| WiFi Password      | ✅       |
| Green timing       | ✅       |
| Yellow timing      | ✅       |
| Red timing         | ✅       |
| Pattern mode       | ✅       |
| Custom light state | ✅       |

All values persist after:

* Power loss
* Reset button
* ESP.restart()

---

# WiFi Behavior

## Normal Mode (STA)

* Connects to saved SSID.
* Web interface available at:

  ```
  http://<device-ip>/
  ```

## Setup Mode (AP Fallback)

If WiFi fails or no credentials are saved:

* Starts Access Point:

  ```
  SSID: TrafficLight-Setup
  Password: trafficlight
  IP: 192.168.4.1
  ```
* Open:

  ```
  http://192.168.4.1/settings
  ```

After saving WiFi credentials, device reboots.

---

# Project File Structure

```
traffic-controller.ino → Main program + state machine
web_control.h          → Web UI and HTTP routes
wifi_helper.h          → WiFi connection + AP fallback logic
wifi_config.h          → NVS storage helpers
```

---

# How It Works

## Main Loop Overview

The `loop()` function:

1. Maintains WiFi
2. Handles HTTP requests
3. Runs traffic state machine (unless in Custom mode)

---

## Pattern Logic

### US Pattern

```
GREEN
  ↓
YELLOW
  ↓
ALL_RED (short clearance)
  ↓
RED
  ↓
ALL_RED
  ↓
GREEN
```

---

### German Pattern

```
RED
  ↓
RED + YELLOW
  ↓
GREEN
  ↓
YELLOW
  ↓
RED
```

---

### Custom Mode

* State machine is bypassed.
* Manual bit flags determine which lights are on.
* Loop continuously applies custom bits.

---

# Custom Mode Bit Layout

Custom light states are stored as bit flags:

| Bit | Light  |
| --- | ------ |
| 0   | Red    |
| 1   | Yellow |
| 2   | Green  |

Example:

```
0b00000101
```

Red + Green ON

---

# Hardware Wiring

## GPIO Pins Used

| GPIO | Light  |
| ---- | ------ |
| 18   | Red    |
| 17   | Yellow |
| 16   | Green  |

These connect to:

* 3-channel MOSFET board
* Common ground required
* Active HIGH trigger assumed

If your MOSFET board is active LOW, change:

```cpp
static const bool ACTIVE_LOW = true;
```

---

# Requirements

* ESP32 Development Board
* 3-channel MOSFET driver board
* 12V or 24V LED traffic lights (depending on your model)
* Arduino IDE with ESP32 board package installed

---

# Installation

1. Install ESP32 board support in Arduino IDE
2. Clone this repository
3. Open `traffic-controller.ino`
4. Upload to ESP32
5. Open Serial Monitor at 115200 baud
6. Navigate to printed IP address

---

# Security Note

This project is designed for hobby or lab use.

It does NOT include:

* HTTPS
* Authentication
* Access control

If exposed outside a private LAN, add authentication.
