#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h>
#include "light_sensor.h"
#include "temp_sensor.h"
#include "led_control.h"
#include "routes_api.h"

#define LDR_PIN        32
#define THERMISTOR_PIN 33
#define LED_PIN        25

const char* ssid = "Imim";
const char* password = "23232324";

AsyncWebServer server(80);
TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);

  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  analogSetPinAttenuation(THERMISTOR_PIN, ADC_11db);

  initLED(LED_PIN);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  registerRoutes(server, LDR_PIN, THERMISTOR_PIN, LED_PIN);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  int adc = readLDR(LDR_PIN);
  float lumPercent = calculateLuminosityPercent(adc);
  float tempC = readTemperature(THERMISTOR_PIN);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 30);
  tft.printf("Lum: %.1f %%", lumPercent);
  tft.setCursor(10, 80);
  tft.printf("Temp: %.2f C", tempC);
  tft.setCursor(10, 130);
  tft.setTextSize(1);
  tft.printf("IP: %s", WiFi.localIP().toString().c_str());

  delay(1000);
}