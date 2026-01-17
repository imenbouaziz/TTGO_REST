#define CONFIG_ASYNC_TCP_USE_LWIP2 1

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <Button2.h>
#include <math.h>

// ---------------------- Pins -------------------------
#define LED_PIN     25
#define LDR_PIN     32
#define THERM_PIN   33
#define BTN_MANUAL  35   // Manual mode button
#define BTN_AUTO    0    // Auto mode button

// ---------------------- WiFi -------------------------
const char* ssid = "iPhone";
const char* password = "23232324";

// ---------------------- Thermistor -------------------
const float Vref = 3.3;
const int Rseries = 10000;
const float beta = 3950;
const float Tnom = 25.0;
const int Rnom = 10000;

// ---------------------- Globals ----------------------
AsyncWebServer server(80);
TFT_eSPI tft;

Button2 btnManual(BTN_MANUAL);
Button2 btnAuto(BTN_AUTO);

bool led_state = false;
bool auto_mode = true;

int light_threshold = 1000;
float temp_threshold = 28.0;
bool light_control_enabled = true;
bool temp_control_enabled = false;

int adc_ldr = 0;
float temperature = 0.0;
float humidity = 0.0;

unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 500;

// For double press detection (optional)
unsigned long lastManualPress = 0;
const unsigned long doublePressInterval = 500; // ms

// ----------------------------------------------------
// LDR
int readLDR(int pin, int samples = 10) {
  uint32_t sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / samples;
}

// ----------------------------------------------------
// Thermistor
float readTemperature(int pin, int samples = 10) {
  uint32_t sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(5);
  }

  int adc = sum / samples;
  if (adc < 10 || adc > 4085) return temperature;

  float V = adc * (Vref / 4095.0);
  if (V < 0.01) return temperature;

  float R = Rseries * (Vref / V - 1);
  float Tk = 1.0 / (log(R / Rnom) / beta + 1.0 / (Tnom + 273.15));
  return Tk - 273.15;
}

// ----------------------------------------------------
// Display
void updateDisplay() {
  tft.fillScreen(TFT_BLACK);

  tft.setCursor(10, 5);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.println("ESP32 IoT Server");

  tft.setCursor(10, 20);
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW);
  tft.print("IP: ");
  tft.println(WiFi.localIP());

  tft.setCursor(10, 45);
  tft.setTextColor(led_state ? TFT_GREEN : TFT_RED);
  tft.printf("LED: %s\n", led_state ? "ON" : "OFF");

  tft.setCursor(10, 65);
  tft.setTextColor(auto_mode ? TFT_GREEN : TFT_ORANGE);
  tft.printf("Mode: %s\n", auto_mode ? "AUTO" : "MANUEL");

  float percent = (adc_ldr / 4095.0) * 100.0;
  tft.setCursor(10, 85);
  tft.setTextColor(TFT_CYAN);
  tft.printf("Lum: %.1f%%\n", percent);

  tft.setCursor(10, 100);
  tft.setTextColor(TFT_ORANGE);
  tft.printf("Temp: %.2f C\n", temperature);
}

// ----------------------------------------------------
void readSensors() {
  adc_ldr = readLDR(LDR_PIN);
  temperature = readTemperature(THERM_PIN);
}

// ----------------------------------------------------
void autoControl() {
  if (!auto_mode) return;

  bool should_on = false;

  if (light_control_enabled && adc_ldr < light_threshold)
    should_on = true;

  if (temp_control_enabled && temperature > temp_threshold)
    should_on = true;

  if (should_on != led_state) {
    led_state = should_on;
    digitalWrite(LED_PIN, led_state ? LOW : HIGH);
  }
}

// ----------------------------------------------------
void setLED(bool state) {
  led_state = state;
  digitalWrite(LED_PIN, led_state ? LOW : HIGH);
  updateDisplay();
}

// ----------------------------------------------------
// Button handlers
void onManualPressed(Button2&) {
  unsigned long now = millis();

  if (!auto_mode) {
    // Already in manual mode → toggle LED
    if (now - lastManualPress > 200) { // debounce 200ms
      led_state = !led_state;
      digitalWrite(LED_PIN, led_state ? LOW : HIGH);
      Serial.printf("MANUAL mode - LED toggled: %s\n", led_state ? "ON" : "OFF");
    }
  } else {
    // Switch to manual mode
    auto_mode = false;
    Serial.println("MANUAL mode (button)");
  }

  lastManualPress = now;
  updateDisplay();
}

void onAutoPressed(Button2&) {
  auto_mode = true;
  updateDisplay();
  Serial.println("AUTO mode (button)");
}

// ----------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(BTN_MANUAL, INPUT);        // GPIO 35
  pinMode(BTN_AUTO, INPUT_PULLUP);   // GPIO 0

  btnManual.setPressedHandler(onManualPressed);
  btnAuto.setPressedHandler(onAutoPressed);

  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  analogSetPinAttenuation(THERM_PIN, ADC_11db);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 30);
  tft.setTextSize(2);
  tft.println("Connecting...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  readSensors();
  updateDisplay();

  // ---------------- Web server ----------------
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *r){
    r->send(200, "text/plain", "ESP32 IoT API");
  });

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *r){
    auto_mode = false;
    setLED(true);
    r->send(200, "application/json", "{\"led\":\"ON\"}");
  });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *r){
    auto_mode = false;
    setLED(false);
    r->send(200, "application/json", "{\"led\":\"OFF\"}");
  });

  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *r){
    auto_mode = false;
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state ? LOW : HIGH);
    updateDisplay();
    r->send(200, "application/json", "{\"led\":\"TOGGLED\"}");
  });

  server.on("/auto/on", HTTP_GET, [](AsyncWebServerRequest *r){
    auto_mode = true;
    updateDisplay();
    r->send(200, "application/json", "{\"mode\":\"AUTO\"}");
  });

  server.begin();
}

// ----------------------------------------------------
void loop() {
  unsigned long now = millis();

  if (now - lastSensorRead >= sensorInterval) {
    lastSensorRead = now;
    readSensors();
    autoControl();
    updateDisplay();
  }

  // REQUIRED for Button2
  btnManual.loop();
  btnAuto.loop();
}
