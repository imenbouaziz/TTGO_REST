#include "light_sensor.h"

int readLDR(int pin, int samples) {
    uint32_t sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
        delay(5);
    }
    return sum / samples;
}

float calculateVoltage(int adcValue) {
    return adcValue * 3.3 / 4095.0;
}

float calculateLuminosityPercent(int adcValue) {
    return (adcValue / 4095.0) * 100.0;
}

