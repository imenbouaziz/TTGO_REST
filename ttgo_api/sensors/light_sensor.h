#pragma once
#include <Arduino.h>

int readLDR(int pin, int samples = 10);
float calculateVoltage(int adcValue);
float calculateLuminosityPercent(int adcValue);
