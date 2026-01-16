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
#define BUTTON_AUTO_PIN 0    // Bouton droit (souvent BOOT)
#define BUTTON_MANUAL_PIN 35 // Bouton gauche

const char* ssid = "Imim";
const char* password = "23232324";

// État global
bool isAutoMode = true;
unsigned long lastUpdate = 0;
const int updateInterval = 1000;

AsyncWebServer server(80);
TFT_eSPI tft = TFT_eSPI();

void setup() {
    Serial.begin(115200);
    
    // Configuration des boutons
    pinMode(BUTTON_AUTO_PIN, INPUT_PULLUP);
    pinMode(BUTTON_MANUAL_PIN, INPUT); // Pin 35 est Input Only, utilise le pullup externe de la carte

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

    // Affichage initial
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(TFT_GREEN);
    tft.println("Mode: AUTO");
}

void loop() {
    // Lecture des boutons
    if (digitalRead(BUTTON_AUTO_PIN) == LOW) {
        if (!isAutoMode) {
            isAutoMode = true;
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(10, 10);
            tft.setTextColor(TFT_GREEN); 
            tft.setTextSize(2);
            tft.println("Mode: AUTO");
            delay(200); // Anti-rebond simple
        }
    }
    
    if (digitalRead(BUTTON_MANUAL_PIN) == LOW) {
        if (isAutoMode) {
            isAutoMode = false;
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(10, 60);
            tft.setTextColor(TFT_RED);
            tft.setTextSize(2);
            tft.println("Mode: MANUEL");
            tft.setCursor(10, 90);
            tft.setTextSize(1);
            tft.println("Controle via API...");
            delay(200); // Anti-rebond simple
        }
    }

    // Comportement Mode Automatique
    if (isAutoMode && (millis() - lastUpdate > updateInterval)) {
        lastUpdate = millis();

        int adc = readLDR(LDR_PIN);
        float tempC = readTemperature(THERMISTOR_PIN);

        // Effacer seulement la zone des données pour éviter le scintillement
        tft.fillRect(0, 30, 240, 135, TFT_BLACK); 
        
        tft.setCursor(10, 10);
        tft.setTextColor(TFT_GREEN);
        tft.setTextSize(2);
        tft.println("Mode: AUTO");

        tft.setCursor(10, 50);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.printf("Lum: %.1f %%\n", calculateLuminosityPercent(adc));
        tft.setCursor(10, 90);
        tft.printf("Temp: %.2f C\n", tempC);
    }
}
