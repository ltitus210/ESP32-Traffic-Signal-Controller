#include <Arduino.h>
#include <WebServer.h>
#include "wifi_helper.h"
#include "web_control.h"

// ===== MOSFET / light pins =====
static const int PIN_RED    = 18;
static const int PIN_YELLOW = 17;
static const int PIN_GREEN  = 16;

// Most MOSFET trigger boards are LOW-level trigger:
static const bool ACTIVE_LOW = false;

// ===== Web server =====
WebServer server(80);

// ===== Timing (editable & persisted) =====
volatile uint32_t GREEN_MS  = 12000;
volatile uint32_t YELLOW_MS = 3000;
volatile uint32_t RED_MS    = 12000;
static const uint32_t ALL_RED_MS = 500;

// ===== Traffic light state machine =====
enum State { ST_GREEN, ST_YELLOW, ST_ALL_RED_1, ST_RED, ST_ALL_RED_2 };
static State state = ST_GREEN;
static uint32_t stateStart = 0;

const char* stateName() {
  switch (state) {
    case ST_GREEN: return "GREEN";
    case ST_YELLOW: return "YELLOW";
    case ST_ALL_RED_1: return "ALL_RED";
    case ST_RED: return "RED";
    case ST_ALL_RED_2: return "ALL_RED";
    default: return "?";
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

void enter(State s) {
  state = s;
  stateStart = millis();
  switch (state) {
    case ST_GREEN:     setLights(false, false, true);  break;
    case ST_YELLOW:    setLights(false, true,  false); break;
    case ST_ALL_RED_1: setLights(true,  false, false); break;
    case ST_RED:       setLights(true,  false, false); break;
    case ST_ALL_RED_2: setLights(true,  false, false); break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  setLights(false, false, false);

  // Load persisted timings (optional but nice)
  loadTimings((uint32_t&)GREEN_MS, (uint32_t&)YELLOW_MS, (uint32_t&)RED_MS);

  // Start WiFi (STA if creds work, otherwise AP setup mode)
  wifiBegin();

  Serial.print("Mode: ");
  Serial.println(isApMode() ? "SETUP AP" : "WIFI STA");

  if (isApMode()) {
    Serial.print("AP SSID: "); Serial.println("TrafficLight-Setup");
    Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
    Serial.println("Open: http://192.168.4.1/settings");
  } else {
    Serial.print("IP: "); Serial.println(WiFi.localIP());
    Serial.println("Open: http://<ip>/");
  }

  setupWebRoutes();
  enter(ST_GREEN);
}

void loop() {
  wifiMaintain();
  server.handleClient();

  uint32_t elapsed = millis() - stateStart;

  switch (state) {
    case ST_GREEN:
      if (elapsed >= (uint32_t)GREEN_MS) enter(ST_YELLOW);
      break;
    case ST_YELLOW:
      if (elapsed >= (uint32_t)YELLOW_MS) enter(ST_ALL_RED_1);
      break;
    case ST_ALL_RED_1:
      if (elapsed >= ALL_RED_MS) enter(ST_RED);
      break;
    case ST_RED:
      if (elapsed >= (uint32_t)RED_MS) enter(ST_ALL_RED_2);
      break;
    case ST_ALL_RED_2:
      if (elapsed >= ALL_RED_MS) enter(ST_GREEN);
      break;
  }
}
