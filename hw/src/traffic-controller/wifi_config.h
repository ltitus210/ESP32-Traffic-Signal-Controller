#pragma once
#include <Preferences.h>

// All values stored under this NVS namespace
static const char* NVS_NAMESPACE = "tl_cfg";

// ===== WiFi Keys =====
static const char* KEY_SSID    = "ssid";
static const char* KEY_PASS    = "pass";

// ===== Timing Keys =====
static const char* KEY_G       = "g_ms";
static const char* KEY_Y       = "y_ms";
static const char* KEY_R       = "r_ms";

// ===== Pattern Key =====
// 0 = United States
// 1 = Germany (Red+Yellow before Green)
// 2 = Custom (manual)
static const char* KEY_PATTERN = "pattern";

// ===== Custom Mode Light Bits =====
// bit0 = Red
// bit1 = Yellow
// bit2 = Green
static const char* KEY_CUSTOM  = "cust_bits";


// ======================================================
//                    WIFI FUNCTIONS
// ======================================================

inline bool loadWifiCreds(String &ssid, String &pass) {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, true)) return false;

  ssid = prefs.getString(KEY_SSID, "");
  pass = prefs.getString(KEY_PASS, "");

  prefs.end();
  return ssid.length() > 0; // SSID must exist
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


// ======================================================
//                   TIMING FUNCTIONS
// ======================================================

inline void loadTimings(uint32_t &gMs, uint32_t &yMs, uint32_t &rMs) {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, true)) return;

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


// ======================================================
//                   PATTERN FUNCTIONS
// ======================================================

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


// ======================================================
//              CUSTOM MODE LIGHT FUNCTIONS
// ======================================================

inline uint8_t loadCustomBits(uint8_t defaultBits = 0) {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, true)) return defaultBits;

  uint8_t v = prefs.getUChar(KEY_CUSTOM, defaultBits);

  prefs.end();
  return v;
}

inline void saveCustomBits(uint8_t bits) {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, false)) return;

  prefs.putUChar(KEY_CUSTOM, bits);

  prefs.end();
}
