#include <Arduino.h>
#include <WebServer.h>

#include "wifi_helper.h"
#include "web_control.h"
#include "wifi_config.h"

// ===== MOSFET / light pins =====
static const int PIN_RED    = 18;
static const int PIN_YELLOW = 17;
static const int PIN_GREEN  = 16;

// Most MOSFET trigger boards are LOW-level trigger:
// IN LOW = ON, IN HIGH = OFF
static const bool ACTIVE_LOW = false;

// ===== Web server =====
WebServer server(80);

// ===== Timing (editable & persisted) =====
volatile uint32_t GREEN_MS  = 12000;
volatile uint32_t YELLOW_MS = 3000;
volatile uint32_t RED_MS    = 12000;

// Optional short all-red clearance (used in US pattern)
static const uint32_t ALL_RED_MS = 500;

// Pattern: 0 = US, 1 = DE (German), 2 = CUSTOM (manual)
volatile uint8_t PATTERN_MODE = 0;

// Custom/manual bits (bit0=R, bit1=Y, bit2=G)
volatile uint8_t CUSTOM_BITS = 0;

// ===== Traffic light state machine =====
enum State { ST_GREEN, ST_YELLOW, ST_ALL_RED, ST_RED, ST_RED_YELLOW };
static State state = ST_GREEN;
static uint32_t stateStart = 0;

// Used only for US pattern to decide where ALL_RED goes next
static bool allRedToGreen = false;

const char* stateName() {
  if (PATTERN_MODE == 2) {
    return "CUSTOM";
  }
  switch (state) {
    case ST_GREEN:      return "GREEN";
    case ST_YELLOW:     return "YELLOW";
    case ST_RED:        return "RED";
    case ST_RED_YELLOW: return "RED+YELLOW";
    case ST_ALL_RED:    return "ALL_RED";
    default:            return "?";
  }
}

inline void writeChannel(int pin, bool on) {
  digitalWrite(pin, ACTIVE_LOW ? (on ? LOW : HIGH) : (on ? HIGH : LOW));
}

void setLights(bool redOn, bool yellowOn, bool greenOn) {
  writeChannel(PIN_RED, redOn);
  writeChannel(PIN_YELLOW, yellowOn);
  writeChannel(PIN_GREEN, greenOn);
}

void applyCustomBits(uint8_t bits) {
  bool r = (bits & 0x01) != 0;
  bool y = (bits & 0x02) != 0;
  bool g = (bits & 0x04) != 0;
  setLights(r, y, g);
}

void enter(State s) {
  state = s;
  stateStart = millis();

  switch (state) {
    case ST_GREEN:      setLights(false, false, true);  break;
    case ST_YELLOW:     setLights(false, true,  false); break;
    case ST_RED:        setLights(true,  false, false); break;
    case ST_RED_YELLOW: setLights(true,  true,  false); break; // German pre-green
    case ST_ALL_RED:    setLights(true,  false, false); break; // single-head "all red"
  }
}

void startPatternFromBeginning() {
  allRedToGreen = false;

  if (PATTERN_MODE == 0) {
    enter(ST_GREEN);
  } else if (PATTERN_MODE == 1) {
    enter(ST_RED);
  } else {
    // Custom/manual
    applyCustomBits(CUSTOM_BITS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);

  // Force all OFF at boot
  setLights(false, false, false);

  // Load persisted settings
  loadTimings((uint32_t&)GREEN_MS, (uint32_t&)YELLOW_MS, (uint32_t&)RED_MS);
  PATTERN_MODE = loadPattern(0);            // 0=US default
  CUSTOM_BITS  = loadCustomBits(0);         // default all off

  // Start WiFi (STA if creds work, otherwise AP setup mode)
  wifiBegin();

  Serial.print("Mode: ");
  Serial.println(isApMode() ? "SETUP AP" : "WIFI STA");

  if (isApMode()) {
    Serial.print("AP SSID: ");
    Serial.println("TrafficLight-Setup");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Open: http://192.168.4.1/settings");
  } else {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("Open: http://<ip>/");
  }

  setupWebRoutes();

  // Start the selected pattern
  startPatternFromBeginning();
}

void loop() {
  // Keep WiFi alive + serve HTTP
  wifiMaintain();
  server.handleClient();

  // Custom mode: no timing/state machine, just hold the chosen outputs
  if (PATTERN_MODE == 2) {
    // Apply any updated bits (in case changed via web)
    applyCustomBits(CUSTOM_BITS);
    delay(5);
    return;
  }

  uint32_t elapsed = millis() - stateStart;

  // Typical German red+yellow duration
  static const uint32_t RED_YELLOW_MS = 1000;

  if (PATTERN_MODE == 0) {
    // ===== US Pattern =====
    // GREEN -> YELLOW -> ALL_RED -> RED -> ALL_RED -> GREEN
    switch (state) {
      case ST_GREEN:
        if (elapsed >= (uint32_t)GREEN_MS) enter(ST_YELLOW);
        break;

      case ST_YELLOW:
        if (elapsed >= (uint32_t)YELLOW_MS) {
          allRedToGreen = false;   // after ALL_RED go to RED
          enter(ST_ALL_RED);
        }
        break;

      case ST_ALL_RED:
        if (elapsed >= ALL_RED_MS) {
          enter(allRedToGreen ? ST_GREEN : ST_RED);
        }
        break;

      case ST_RED:
        if (elapsed >= (uint32_t)RED_MS) {
          allRedToGreen = true;    // after ALL_RED go to GREEN
          enter(ST_ALL_RED);
        }
        break;

      default:
        enter(ST_GREEN);
        break;
    }
  } else {
    // ===== German Pattern =====
    // RED -> RED+YELLOW -> GREEN -> YELLOW -> RED
    switch (state) {
      case ST_RED:
        if (elapsed >= (uint32_t)RED_MS) enter(ST_RED_YELLOW);
        break;

      case ST_RED_YELLOW:
        if (elapsed >= RED_YELLOW_MS) enter(ST_GREEN);
        break;

      case ST_GREEN:
        if (elapsed >= (uint32_t)GREEN_MS) enter(ST_YELLOW);
        break;

      case ST_YELLOW:
        if (elapsed >= (uint32_t)YELLOW_MS) enter(ST_RED);
        break;

      default:
        enter(ST_RED);
        break;
    }
  }
}
