#pragma once
#include <WiFi.h>
#include "wifi_config.h"
#include "secrets.h"

static const char* WIFI_HOSTNAME = "esp32-traffic-light";

// If Wi-Fi fails, we start a setup AP so you can still reach the UI.
static const char* SETUP_AP_SSID = "TrafficLight-Setup";
static const char* SETUP_AP_PASS = "trafficlight"; // >=8 chars, or set "" for open AP

static bool g_isApMode = false;
static unsigned long g_lastReconnectAttempt = 0;

inline bool isApMode() { return g_isApMode; }

inline bool wifiConnectStored(uint32_t timeoutMs = 10000) {
  String ssid, pass;
  if (!loadWifiCreds(ssid, pass)) return false;

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(WIFI_HOSTNAME);
  WiFi.begin(ssid.c_str(), pass.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
    delay(250);
  }

  return WiFi.status() == WL_CONNECTED;
}

inline void startSetupAP() {
  g_isApMode = true;

  WiFi.mode(WIFI_AP);
  if (strlen(SETUP_AP_PASS) >= 8) {
    WiFi.softAP(SETUP_AP_SSID, SETUP_AP_PASS);
  } else {
    WiFi.softAP(SETUP_AP_SSID); // open AP
  }
}

inline void wifiBegin() {
  g_isApMode = false;

  if (wifiConnectStored(12000)) {
    // connected
    return;
  }

  // Couldn't connect (or no creds) → AP setup mode
  startSetupAP();
}

// Call frequently in loop() to auto-reconnect if STA drops
inline void wifiMaintain() {
  if (g_isApMode) return; // In AP mode, stay there until user changes creds

  if (WiFi.status() == WL_CONNECTED) return;

  // Retry every 10 seconds
  if (millis() - g_lastReconnectAttempt < 10000) return;
  g_lastReconnectAttempt = millis();

  // Try stored creds again
  wifiConnectStored(8000);
}
