#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h>

#include "sensors/light_sensor.h"
#include "sensors/temp_sensor.h"
#include "led/led_controller.h"
#include "api/api_routes.h"

#define LDR_PIN 32
#define THERMISTOR_PIN 33
#define LED_PIN 25

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
    Serial.println(WiFi.localIP());

    registerRoutes(server, LDR_PIN, THERMISTOR_PIN, LED_PIN);
    server.begin();
}

void loop() {
    int adc = readLDR(LDR_PIN);
    float tempC = readTemperature(THERMISTOR_PIN);

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 30);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("Lum: %.1f %%\n", calculateLuminosityPercent(adc));
    tft.setCursor(10, 70);
    tft.printf("Temp: %.2f C\n", tempC);

    delay(1000);
}
