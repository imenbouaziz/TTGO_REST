#define CONFIG_ASYNC_TCP_USE_LWIP2 1
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h>

const char* ssid = "Imim";
const char* password = "23232324";

//config thermistor-------------------------------------------------------------------------
#define THERMISTOR_PIN 32   // ADC pin connected to thermistor divider
const float referenceVoltage = 3.3;
const int seriesResistor = 10000;     // 10kΩ fixed resistor
const float beta = 3950;              // Beta constant of thermistor
const float tempNominal = 25.0;       // Nominal temperature (25°C)
const int resistanceNominal = 10000;  // Resistance at 25°C (10ko)

//----------------
AsyncWebServer server(80);
TFT_eSPI tft = TFT_eSPI();

//-----------------------------------------------------------------------
float readTemperature(int pin, int samples = 10) {
  uint32_t sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  int adcValue = sum / samples;

  float voltage = adcValue * (referenceVoltage / 4095.0);
  float resistance = seriesResistor * (referenceVoltage / voltage - 1);

  float temperatureK = 1.0 / (log(resistance / resistanceNominal) / beta + 1.0 / (tempNominal + 273.15));
  float temperatureC = temperatureK - 273.15;

  return temperatureC;
}

void updateTemperatureDisplay(float tempC) {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 30);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("Temp: %.2f C\n", tempC);

  tft.setCursor(10, 70);
  tft.printf("WiFi: %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  analogReadResolution(12);
  analogSetPinAttenuation(THERMISTOR_PIN, ADC_11db);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  tft.setCursor(10, 150);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("IP: ");
  tft.println(WiFi.localIP());

  //les routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "API OK");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    float tempC = readTemperature(THERMISTOR_PIN);
    String response = "Temperature = " + String(tempC, 2) + " C\n";
    request->send(200, "text/plain", response);
  });

  server.begin();
  Serial.println("Server started");
}

void loop() {
  float tempC = readTemperature(THERMISTOR_PIN);
  updateTemperatureDisplay(tempC);
  delay(1000);
}
