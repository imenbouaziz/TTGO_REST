#include "led_control.h"

static int ledPin = -1;

void initLED(int pin) {
  ledPin = pin;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void setLEDState(int pin, bool on) {
  ledPin = pin;
  digitalWrite(pin, on ? HIGH : LOW);
}

bool getLEDState(int pin) {
  if (ledPin != pin) return false;
  return digitalRead(pin) == HIGH;
}