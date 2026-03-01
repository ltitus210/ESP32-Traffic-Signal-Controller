#pragma once
#include <WebServer.h>
#include "wifi_config.h"
#include "wifi_helper.h"

extern WebServer server;
extern volatile uint32_t GREEN_MS;
extern volatile uint32_t YELLOW_MS;
extern volatile uint32_t RED_MS;
extern volatile uint8_t PATTERN_MODE;
extern volatile uint8_t CUSTOM_BITS;
extern const char* stateName();

// Bit masks for custom mode
static const uint8_t BIT_R = 0x01;
static const uint8_t BIT_Y = 0x02;
static const uint8_t BIT_G = 0x04;

inline uint32_t clampU32(uint32_t v, uint32_t lo, uint32_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static String pageHeader(const char* title) {
  String s;
  s.reserve(2500);
  s += "<!doctype html><html><head>";
  s += "<meta charset='utf-8'>";
  s += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  s += "<title>"; s += title; s += "</title>";
  s += "<style>";
  s += "body{font-family:system-ui;margin:20px;max-width:640px}";
  s += ".box{padding:14px;border:1px solid #ddd;border-radius:12px;margin:12px 0}";
  s += "label{display:block;margin:12px 0 6px}";
  s += "input[type=range]{width:100%}";
  s += "select{width:100%;padding:10px;border-radius:10px;border:1px solid #ccc;font-size:16px}";
  s += "button{width:100%;padding:10px;border-radius:10px;border:1px solid #ccc;font-size:16px;background:#f7f7f7;margin-top:10px}";
  s += ".row{display:flex;gap:10px;margin-top:10px}";
  s += ".row button{flex:1;margin-top:0}";
  s += ".small{color:#555;font-size:14px}";
  s += ".nav{margin-bottom:10px}";
  s += ".pill{display:inline-block;padding:2px 8px;border:1px solid #ddd;border-radius:999px;font-size:12px;margin-left:8px}";
  s += "</style></head><body>";
  s += "<div class='small nav'><a href='/'>Timing</a> | <a href='/settings'>WiFi Settings</a></div>";
  return s;
}

static String pageFooter() { return "</body></html>"; }

static String onOffPill(bool on) {
  return on ? "<span class='pill'>ON</span>" : "<span class='pill'>OFF</span>";
}

// ---------- MAIN PAGE ----------
inline void handleRoot() {
  uint32_t g = GREEN_MS / 1000;
  uint32_t y = YELLOW_MS / 1000;
  uint32_t r = RED_MS / 1000;

  bool cr = (CUSTOM_BITS & BIT_R) != 0;
  bool cy = (CUSTOM_BITS & BIT_Y) != 0;
  bool cg = (CUSTOM_BITS & BIT_G) != 0;

  String s = pageHeader("Traffic Light");

  s += "<h2>Traffic Light Controller</h2>";
  s += "<div class='box'>";
  s += "<div><b>State:</b> <span id='st'>...</span></div>";
  s += "<div class='small'>Choose a pattern. In Custom mode, timings are ignored and you control lights manually.</div>";

  s += "<form method='POST' action='/set'>";

  // Pattern selector
  s += "<label>Signal Pattern</label>";
  s += "<select name='p'>";
  s += "<option value='0'"; if (PATTERN_MODE == 0) s += " selected"; s += ">United States</option>";
  s += "<option value='1'"; if (PATTERN_MODE == 1) s += " selected"; s += ">Germany (Red+Yellow before Green)</option>";
  s += "<option value='2'"; if (PATTERN_MODE == 2) s += " selected"; s += ">Custom (Manual)</option>";
  s += "</select>";

  // Sliders only relevant when not custom (still shown; custom ignores them)
  s += "<label>Green: <span id='gval'>" + String(g) + "</span>s (1-60)</label>";
  s += "<input name='g' type='range' min='1' max='60' value='" + String(g) + "' oninput='document.getElementById(\"gval\").textContent=this.value'>";

  s += "<label>Yellow: <span id='yval'>" + String(y) + "</span>s (1-10)</label>";
  s += "<input name='y' type='range' min='1' max='10' value='" + String(y) + "' oninput='document.getElementById(\"yval\").textContent=this.value'>";

  s += "<label>Red: <span id='rval'>" + String(r) + "</span>s (1-60)</label>";
  s += "<input name='r' type='range' min='1' max='60' value='" + String(r) + "' oninput='document.getElementById(\"rval\").textContent=this.value'>";

  s += "<button type='submit'>Apply</button>";
  s += "</form>";
  s += "</div>";

  // Custom controls shown only in Custom mode
  s += "<div class='box' id='customBox' style='display:";
  s += (PATTERN_MODE == 2 ? "block" : "none");
  s += "'>";
  s += "<h3>Custom Manual Control</h3>";
  s += "<div class='small'>Toggle each light on/off. These settings persist after reboot.</div>";

  // Each button posts to /custom with current desired bits
  s += "<form method='POST' action='/custom'>";
  s += "<label>Red " + onOffPill(cr) + "</label>";
  s += "<div class='row'>";
  s += "<button name='set' value='r_on'>Red ON</button>";
  s += "<button name='set' value='r_off'>Red OFF</button>";
  s += "</div>";

  s += "<label>Yellow " + onOffPill(cy) + "</label>";
  s += "<div class='row'>";
  s += "<button name='set' value='y_on'>Yellow ON</button>";
  s += "<button name='set' value='y_off'>Yellow OFF</button>";
  s += "</div>";

  s += "<label>Green " + onOffPill(cg) + "</label>";
  s += "<div class='row'>";
  s += "<button name='set' value='g_on'>Green ON</button>";
  s += "<button name='set' value='g_off'>Green OFF</button>";
  s += "</div>";

  s += "<label>Quick Actions</label>";
  s += "<div class='row'>";
  s += "<button name='set' value='all_off'>All OFF</button>";
  s += "<button name='set' value='all_on'>All ON</button>";
  s += "</div>";

  s += "</form>";
  s += "</div>";

  // Live poll updates state and shows/hides custom controls based on pattern
  s += "<script>";
  s += "async function poll(){";
  s += "try{let r=await fetch('/status');let j=await r.json();";
  s += "document.getElementById('st').textContent=j.state+' (G='+j.green+'s, Y='+j.yellow+'s, R='+j.red+'s)';";
  s += "document.getElementById('customBox').style.display=(j.pattern==2?'block':'none');";
  s += "}catch(e){}";
  s += "setTimeout(poll,1000);";
  s += "}";
  s += "poll();";
  s += "</script>";

  s += pageFooter();
  server.send(200, "text/html", s);
}

inline void handleStatus() {
  String json = "{";
  json += "\"state\":\"" + String(stateName()) + "\",";
  json += "\"pattern\":" + String((int)PATTERN_MODE) + ",";
  json += "\"green\":" + String(GREEN_MS / 1000) + ",";
  json += "\"yellow\":" + String(YELLOW_MS / 1000) + ",";
  json += "\"red\":" + String(RED_MS / 1000) + ",";
  json += "\"custom\":" + String((int)CUSTOM_BITS);
  json += "}";
  server.send(200, "application/json", json);
}

inline void handleSet() {
  uint32_t g = server.hasArg("g") ? server.arg("g").toInt() : (GREEN_MS / 1000);
  uint32_t y = server.hasArg("y") ? server.arg("y").toInt() : (YELLOW_MS / 1000);
  uint32_t r = server.hasArg("r") ? server.arg("r").toInt() : (RED_MS / 1000);
  uint32_t p = server.hasArg("p") ? server.arg("p").toInt() : PATTERN_MODE;

  g = clampU32(g, 1, 60);
  y = clampU32(y, 1, 10);
  r = clampU32(r, 1, 60);
  p = clampU32(p, 0, 2);

  GREEN_MS  = g * 1000UL;
  YELLOW_MS = y * 1000UL;
  RED_MS    = r * 1000UL;

  PATTERN_MODE = (uint8_t)p;

  // Persist pattern always
  savePattern(PATTERN_MODE);

  // Persist timings only when not custom (optional, but usually what you want)
  if (PATTERN_MODE != 2) {
    saveTimings(GREEN_MS, YELLOW_MS, RED_MS);
  }

  server.sendHeader("Location", "/");
  server.send(303);
}

// Handle custom toggles (only meaningful in Custom mode)
inline void handleCustom() {
  if (!server.hasArg("set")) {
    server.sendHeader("Location", "/");
    server.send(303);
    return;
  }

  String cmd = server.arg("set");

  uint8_t bits = (uint8_t)CUSTOM_BITS;

  if (cmd == "r_on") bits |= BIT_R;
  else if (cmd == "r_off") bits &= ~BIT_R;
  else if (cmd == "y_on") bits |= BIT_Y;
  else if (cmd == "y_off") bits &= ~BIT_Y;
  else if (cmd == "g_on") bits |= BIT_G;
  else if (cmd == "g_off") bits &= ~BIT_G;
  else if (cmd == "all_off") bits = 0;
  else if (cmd == "all_on") bits = (BIT_R | BIT_Y | BIT_G);

  CUSTOM_BITS = bits;
  saveCustomBits(bits);

  // Ensure we stay in Custom mode if using these buttons
  PATTERN_MODE = 2;
  savePattern(PATTERN_MODE);

  server.sendHeader("Location", "/");
  server.send(303);
}

// ---------- WIFI SETTINGS PAGE ----------
inline void handleSettings() {
  String curSsid, curPass;
  loadWifiCreds(curSsid, curPass);

  String s = pageHeader("WiFi Settings");
  s += "<h2>WiFi Settings</h2>";
  s += "<div class='box'>";
  s += "<form method='POST' action='/wifi-save'>";
  s += "<label>SSID</label>";
  s += "<input name='ssid' type='text' value='" + curSsid + "'>";
  s += "<label>Password</label>";
  s += "<input name='pass' type='password' value='" + curPass + "'>";
  s += "<button type='submit'>Save and Reboot</button>";
  s += "</form>";

  s += "<form method='POST' action='/wifi-clear'>";
  s += "<button type='submit'>Clear WiFi and Reboot (Setup AP)</button>";
  s += "</form>";
  s += "</div>";

  s += pageFooter();
  server.send(200, "text/html", s);
}

inline void handleWifiSave() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");

  ssid.trim();
  if (ssid.length() == 0) {
    server.send(400, "text/plain", "SSID cannot be empty.");
    return;
  }

  saveWifiCreds(ssid, pass);

  server.send(200, "text/html",
              pageHeader("Saved") +
              "<h2>Saved</h2><div class='box'>WiFi saved. Rebooting...</div>" +
              pageFooter());

  delay(500);
  ESP.restart();
}

inline void handleWifiClear() {
  clearWifiCreds();

  server.send(200, "text/html",
              pageHeader("Cleared") +
              "<h2>Cleared</h2><div class='box'>WiFi cleared. Rebooting...</div>" +
              pageFooter());

  delay(500);
  ESP.restart();
}

inline void setupWebRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/set", HTTP_POST, handleSet);

  server.on("/custom", HTTP_POST, handleCustom);

  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/wifi-save", HTTP_POST, handleWifiSave);
  server.on("/wifi-clear", HTTP_POST, handleWifiClear);

  server.begin();
}
