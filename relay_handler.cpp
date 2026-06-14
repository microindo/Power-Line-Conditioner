#include "relay_handler.h"

const uint8_t relayPins[4] = {PIN_RELAY_1, PIN_RELAY_2, PIN_RELAY_3, PIN_RELAY_4};
const uint8_t kontaktorPins[4] = {PIN_KONTAKTOR_1, PIN_KONTAKTOR_2, PIN_KONTAKTOR_3, PIN_KONTAKTOR_4};

bool relayState[4] = {false, false, false, false};
bool kontaktorState[4] = {false, false, false, false};

static unsigned long lastDebounce[4] = {0, 0, 0, 0};
static bool lastRaw[4] = {HIGH, HIGH, HIGH, HIGH};
const unsigned long debounceDelay = 50;

void initRelay() {
  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
    relayState[i] = false;

    pinMode(kontaktorPins[i], INPUT_PULLUP);
    lastRaw[i] = digitalRead(kontaktorPins[i]);
    kontaktorState[i] = !lastRaw[i];
  }
}

void setRelay(uint8_t index, bool state) {
  if (index >= 4) return;
  relayState[index] = state;
  digitalWrite(relayPins[index], state ? HIGH : LOW);
}

void readKontaktor() {
  for (int i = 0; i < 4; i++) {
    bool raw = digitalRead(kontaktorPins[i]);
    if (raw != lastRaw[i]) {
      lastDebounce[i] = millis();
      lastRaw[i] = raw;
    }
    if ((millis() - lastDebounce[i]) > debounceDelay) {
      kontaktorState[i] = !raw;
    }
  }
}

void printRelayStatus() {
  Serial.print("Relay: ");
  for (int i = 0; i < 4; i++) {
    Serial.print("R"); Serial.print(i + 1); Serial.print("=");
    Serial.print(relayState[i] ? "ON " : "OFF ");
  }
  Serial.println();
}

void printKontaktorStatus() {
  Serial.print("Kontaktor: ");
  for (int i = 0; i < 4; i++) {
    Serial.print("C"); Serial.print(i + 1); Serial.print("=");
    Serial.print(kontaktorState[i] ? "ON " : "OFF ");
  }
  Serial.println();
}
