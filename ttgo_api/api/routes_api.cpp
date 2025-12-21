#include "routes_api.h"
#include "light_sensor.h"
#include "temp_sensor.h"
#include "led_control.h"
#include <ArduinoJson.h>

void registerRoutes(AsyncWebServer &server, int ldrPin, int tempPin, int ledPin) {

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "ESP32 Smart Home API OK");
  });

  server.on("/luminosity", HTTP_GET, [ldrPin](AsyncWebServerRequest *request) {
    int adc = readLDR(ldrPin);
    float percent = calculateLuminosityPercent(adc);
    float voltage = calculateVoltage(adc);

    String txt = "ADC: " + String(adc) + "\n";
    txt += "Voltage: " + String(voltage, 2) + " V\n";
    txt += "Luminosity: " + String(percent, 1) + " %";
    request->send(200, "text/plain", txt);
  });

  server.on("/temperature", HTTP_GET, [tempPin](AsyncWebServerRequest *request) {
    float temp = readTemperature(tempPin);
    request->send(200, "text/plain", "Temperature: " + String(temp, 2) + " °C");
  });

  server.on("/led", HTTP_POST, [ledPin](AsyncWebServerRequest *request) {
    if (request->hasParam("state", true)) {
      String val = request->getParam("state", true)->value();
      bool on = (val == "on" || val == "true" || val == "1");
      setLEDState(ledPin, on);

      DynamicJsonDocument doc(200);
      doc["success"] = true;
      doc["led_on"] = on;
      String json;
      serializeJson(doc, json);
      request->send(200, "application/json", json);
    } else {
      request->send(400, "application/json", "{\"error\":\"missing state\"}");
    }
  });

  server.on("/api/status", HTTP_GET, [ldrPin, tempPin, ledPin](AsyncWebServerRequest *request) {
    int adc = readLDR(ldrPin);
    float lum = calculateLuminosityPercent(adc);
    float temp = readTemperature(tempPin);
    bool ledOn = getLEDState(ledPin);

    DynamicJsonDocument doc(512);
    doc["luminosity_percent"] = rounded(lum * 10) / 10.0;
    doc["luminosity_adc"] = adc;
    doc["temperature_c"] = rounded(temp * 100) / 100.0;
    doc["led_on"] = ledOn;

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });
}