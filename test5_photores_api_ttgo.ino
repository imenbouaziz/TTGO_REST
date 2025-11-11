#define CONFIG_ASYNC_TCP_USE_LWIP2 1
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h>

#define LDR_PIN 32

const char* ssid = "Imim";
const char* password = "23232324";

AsyncWebServer server(80);
TFT_eSPI tft = TFT_eSPI();

//-------------------------------------------------------
int readLDR(int pin, int samples = 10) {
  uint32_t sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / samples;
}

//mise à jour de l'affichage tft ttgo
void updateLuminosityDisplay(int adcValue) {
  float voltage = adcValue * 3.3 / 4095.0;
  float percent = (adcValue / 4095.0) * 100.0;

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 30);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("ADC: %d\n", adcValue);

  tft.setCursor(10, 70);
  tft.printf("V: %.2f V\n", voltage);

  tft.setCursor(10, 110);
  tft.printf("Lum: %.1f %%\n", percent);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  analogReadResolution(12); 
  analogSetPinAttenuation(LDR_PIN, ADC_11db);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  WiFi.begin(ssid, password);
  Serial.print("connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nwiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  tft.setCursor(10, 150);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("IP: ");
  tft.println(WiFi.localIP());

  //les routes web------------------------------------------------------------------------
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "API OK");
  });

  server.on("/luminosity", HTTP_GET, [](AsyncWebServerRequest *request){
    int adc = readLDR(LDR_PIN);
    float voltage = adc * 3.3 / 4095.0;
    float percent = (adc / 4095.0) * 100.0;

    String response = "ADC = " + String(adc) + "\n";
    response += "Voltage = " + String(voltage, 2) + " V\n";
    response += "Luminosite = " + String(percent, 1) + " %\n";
    request->send(200, "text/plain", response);
  });

  server.begin();
  Serial.println("server started");
}

void loop() {
  int adc = readLDR(LDR_PIN);
  updateLuminosityDisplay(adc);
  delay(500);
}
