#pragma once
#include <WebServer.h>
#include "wifi_config.h"
#include "wifi_helper.h"

extern WebServer server;
extern volatile uint32_t GREEN_MS;
extern volatile uint32_t YELLOW_MS;
extern volatile uint32_t RED_MS;
extern volatile uint8_t PATTERN_MODE;
extern const char* stateName();

inline uint32_t clampU32(uint32_t v, uint32_t lo, uint32_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static String pageHeader(const char* title) {
  String s;
  s.reserve(2000);
  s += "<!doctype html><html><head>";
  s += "<meta charset='utf-8'>";
  s += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  s += "<title>";
  s += title;
  s += "</title>";
  s += "<style>";
  s += "body{font-family:system-ui;margin:20px;max-width:600px}";
  s += ".box{padding:14px;border:1px solid #ddd;border-radius:12px;margin:12px 0}";
  s += "label{display:block;margin:12px 0 6px}";
  s += "input[type=range]{width:100%}";
  s += "select{width:100%;padding:10px;border-radius:10px;border:1px solid #ccc;font-size:16px}";
  s += "button{width:100%;padding:10px;border-radius:10px;border:1px solid #ccc;font-size:16px;background:#f7f7f7;margin-top:10px}";
  s += ".small{color:#555;font-size:14px}";
  s += ".nav{margin-bottom:10px}";
  s += "</style></head><body>";
  s += "<div class='small nav'><a href='/'>Timing</a> | <a href='/settings'>WiFi Settings</a></div>";
  return s;
}

static String pageFooter() {
  return "</body></html>";
}

inline void handleRoot() {
  uint32_t g = GREEN_MS / 1000;
  uint32_t y = YELLOW_MS / 1000;
  uint32_t r = RED_MS / 1000;

  String s = pageHeader("Traffic Light");

  s += "<h2>Traffic Light Controller</h2>";
  s += "<div class='box'>";
  s += "<div><b>State:</b> <span id='st'>...</span></div>";
  s += "<div class='small'>Adjust timing (seconds) and click Apply.</div>";

  s += "<form method='POST' action='/set'>";

  // Pattern selector
  s += "<label>Signal Pattern</label>";
  s += "<select name='p'>";
  s += "<option value='0'";
  if (PATTERN_MODE == 0) s += " selected";
  s += ">United States</option>";
  s += "<option value='1'";
  if (PATTERN_MODE == 1) s += " selected";
  s += ">Germany (Red+Yellow before Green)</option>";
  s += "</select>";

  // Green
  s += "<label>Green: <span id='gval'>";
  s += String(g);
  s += "</span>s (1-60)</label>";
  s += "<input name='g' type='range' min='1' max='60' value='";
  s += String(g);
  s += "' oninput='document.getElementById(\"gval\").textContent=this.value'>";

  // Yellow
  s += "<label>Yellow: <span id='yval'>";
  s += String(y);
  s += "</span>s (1-10)</label>";
  s += "<input name='y' type='range' min='1' max='10' value='";
  s += String(y);
  s += "' oninput='document.getElementById(\"yval\").textContent=this.value'>";

  // Red
  s += "<label>Red: <span id='rval'>";
  s += String(r);
  s += "</span>s (1-60)</label>";
  s += "<input name='r' type='range' min='1' max='60' value='";
  s += String(r);
  s += "' oninput='document.getElementById(\"rval\").textContent=this.value'>";

  s += "<button type='submit'>Apply</button>";
  s += "</form>";
  s += "</div>";

  // Live status polling
  s += "<script>";
  s += "async function poll(){";
  s += "try{let r=await fetch('/status');let j=await r.json();";
  s += "document.getElementById('st').textContent=j.state+' (G='+j.green+'s, Y='+j.yellow+'s, R='+j.red+'s)';";
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
  json += "\"green\":" + String(GREEN_MS / 1000) + ",";
  json += "\"yellow\":" + String(YELLOW_MS / 1000) + ",";
  json += "\"red\":" + String(RED_MS / 1000);
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
  p = clampU32(p, 0, 1);

  GREEN_MS  = g * 1000UL;
  YELLOW_MS = y * 1000UL;
  RED_MS    = r * 1000UL;
  PATTERN_MODE = (uint8_t)p;

  saveTimings(GREEN_MS, YELLOW_MS, RED_MS);
  savePattern(PATTERN_MODE);

  server.sendHeader("Location", "/");
  server.send(303);
}

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
  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/wifi-save", HTTP_POST, handleWifiSave);
  server.on("/wifi-clear", HTTP_POST, handleWifiClear);
  server.begin();
}
