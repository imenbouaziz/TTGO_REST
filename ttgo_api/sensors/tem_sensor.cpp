// temp_sensor.cpp
#include "temp_sensor.h"
#include <math.h>

const float referenceVoltage = 3.3;
const int seriesResistor = 10000;
const float beta = 3950.0;
const float tempNominal = 25.0;
const int resistanceNominal = 10000;

float readTemperature(int pin, int samples) {
  uint32_t sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  int adcValue = sum / samples;

  if (adcValue == 0) return -999; // erreur

  float voltage = adcValue * (referenceVoltage / 4095.0);
  float resistance = seriesResistor * (referenceVoltage / voltage - 1.0);

  float tempK = 1.0 / (log(resistance / resistanceNominal) / beta + 1.0 / (tempNominal + 273.15));
  return tempK - 273.15;
}