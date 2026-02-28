// ESP32 Traffic Signal Controller
//
// I am using the ESP32 with a Relay or MOSFET board so that the low powered 3.3V logic signals
// of the GPIO Pins can control higher voltage AC/DC traffic light modules directly.
//
// IMPORTANT: Many Relay/MOSFET trigger boards are "LOW-level trigger":
//   IN = LOW  -> ON
//   IN = HIGH -> OFF
//
// Mine happens to be HIGH-level trigger. If yours is LOW-level trigger, set ACTIVE_LOW to true.

// I'm using GPIO pins 16-18. The lables on the silkscreen may differ based on the ESP32 devkit manufacture and revision.
// My pinout: https://lastminuteengineers.com/esp32-pinout-reference/#esp32-gpio-pins
static const int PIN_RED    = 18;   // D18 on silkscreen
static const int PIN_YELLOW = 17;   // TX2 on silkscreen
static const int PIN_GREEN  = 16;   // RX2 on silkscreen

static const bool ACTIVE_LOW = false;   // <-- change to true if your board is low-trigger

// Typical-ish US timing
static const uint32_t GREEN_MS   = 10000;
static const uint32_t YELLOW_MS  = 3000;
static const uint32_t RED_MS     = 10000;
static const uint32_t ALL_RED_MS = 500; // Delayed Green

enum State { GREEN, YELLOW, ALL_RED_1, RED, ALL_RED_2 };
State state = GREEN;
uint32_t stateStart = 0;

inline void writeChannel(int pin, bool on) {
  // active-low: ON=LOW, OFF=HIGH
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
    case GREEN:      setLights(false, false, true);  break;
    case YELLOW:     setLights(false, true,  false); break;
    case ALL_RED_1:  setLights(true,  false, false); break;
    case RED:        setLights(true,  false, false); break;
    case ALL_RED_2:  setLights(true,  false, false); break;
  }
}

void setup() {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);

  // Force all OFF at boot
  setLights(false, false, false);

  enter(GREEN);
}

void loop() {
  uint32_t elapsed = millis() - stateStart;

  switch (state) {
    case GREEN:
      if (elapsed >= GREEN_MS) enter(YELLOW);
      break;

    case YELLOW:
      if (elapsed >= YELLOW_MS) enter(ALL_RED_1);
      break;

    case ALL_RED_1:
      if (elapsed >= ALL_RED_MS) enter(RED);
      break;

    case RED:
      if (elapsed >= RED_MS) enter(ALL_RED_2);
      break;

    case ALL_RED_2:
      if (elapsed >= ALL_RED_MS) enter(GREEN);
      break;
  }
}
