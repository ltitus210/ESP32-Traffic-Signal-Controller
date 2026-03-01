#pragma once
#include <Preferences.h>

static const char* NVS_NAMESPACE = "tl_cfg";
static const char* KEY_SSID = "ssid";
static const char* KEY_PASS = "pass";

// Optional: store timings too (nice to persist)
static const char* KEY_G = "g_ms";
static const char* KEY_Y = "y_ms";
static const char* KEY_R = "r_ms";

static const char* KEY_PATTERN = "pattern"; // 0 = US, 1 = DE

inline uint8_t loadPattern(uint8_t defaultVal = 0) {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, true)) return defaultVal;
  uint8_t v = prefs.getUChar(KEY_PATTERN, defaultVal);
  prefs.end();
  return v;
}

inline void savePattern(uint8_t pattern) {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, false)) return;
  prefs.putUChar(KEY_PATTERN, pattern);
  prefs.end();
}

inline bool loadWifiCreds(String &ssid, String &pass) {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, true)) return false;

  ssid = prefs.getString(KEY_SSID, "");
  pass = prefs.getString(KEY_PASS, "");
  prefs.end();

  return ssid.length() > 0; // password can be empty on open networks
}

inline void saveWifiCreds(const String &ssid, const String &pass) {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, false)) return;

  prefs.putString(KEY_SSID, ssid);
  prefs.putString(KEY_PASS, pass);
  prefs.end();
}

inline void clearWifiCreds() {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, false)) return;
  prefs.remove(KEY_SSID);
  prefs.remove(KEY_PASS);
  prefs.end();
}

inline void loadTimings(uint32_t &gMs, uint32_t &yMs, uint32_t &rMs) {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, true)) return;

  // Keep your existing defaults if missing
  gMs = prefs.getUInt(KEY_G, gMs);
  yMs = prefs.getUInt(KEY_Y, yMs);
  rMs = prefs.getUInt(KEY_R, rMs);

  prefs.end();
}

inline void saveTimings(uint32_t gMs, uint32_t yMs, uint32_t rMs) {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, false)) return;
  prefs.putUInt(KEY_G, gMs);
  prefs.putUInt(KEY_Y, yMs);
  prefs.putUInt(KEY_R, rMs);
  prefs.end();
}
