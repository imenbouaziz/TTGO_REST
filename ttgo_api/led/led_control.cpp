#include "led_controller.h"

void initLED(int pin) {
    pinMode(pin, OUTPUT);
}

void setLEDState(int pin, bool state) {
    digitalWrite(pin, state ? HIGH : LOW);
}
