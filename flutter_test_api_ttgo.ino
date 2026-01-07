#define CONFIG_ASYNC_TCP_USE_LWIP2 1
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>

//---------------------- Pins -------------------------
#define LED_PIN 25
#define LDR_PIN 32
#define THERM_PIN 33

//---------------------- WiFi -------------------------
const char* ssid = "iPhone";
const char* password = "23232324";

//---------------------- Thermistor config ------------
const float Vref = 3.3;
const int Rseries = 10000;          // 10kΩ
const float beta = 3950;
const float Tnom = 25.0;
const int Rnom = 10000;

//---------------------- Globals -----------------------
AsyncWebServer server(80);
TFT_eSPI tft = TFT_eSPI();

// LED State
bool led_state = false;
bool auto_mode = true;  // Mode automatique activé par défaut

// Thresholds
int light_threshold = 1000;         // Seuil luminosité (ADC)
float temp_threshold = 28.0;        // Seuil température (°C)
bool light_control_enabled = true;  // Contrôle par luminosité activé
bool temp_control_enabled = false;  // Contrôle par température désactivé

// Sensor readings
int adc_ldr = 0;
float temperature = 0.0;
float humidity = 0.0;  // Non utilisé mais gardé pour compatibilité API

// Timing
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 500; // Lire toutes les 500ms

//------------------------------------------------------
// Read LDR with averaging
int readLDR(int pin, int samples = 10) {
  uint32_t sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / samples;
}

//------------------------------------------------------
// Read thermistor temperature
float readTemperature(int pin, int samples = 10) {
  uint32_t sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  int adc = sum / samples;
  
  // Vérifier la validité de la lecture
  if (adc < 10 || adc > 4085) {
    return temperature; // Retourner la dernière valeur valide
  }
  
  float V = adc * (Vref / 4095.0);
  if (V < 0.01) return temperature;
  
  float R = Rseries * (Vref / V - 1);
  float Tk = 1.0 / (log(R / Rnom) / beta + 1.0 / (Tnom + 273.15));
  return Tk - 273.15;
}

//------------------------------------------------------
// Update TFT display
//------------------------------------------------------
// Update TFT display - VERSION AMÉLIORÉE
void updateDisplay() {
  float percent = (adc_ldr / 4095.0) * 100.0;
  
  tft.fillScreen(TFT_BLACK);
  
  // ========== TITRE ==========
  tft.setCursor(10, 5);
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.println("ESP32 IoT Server");
  
  // ========== ADRESSE IP (BIEN VISIBLE) ==========
  tft.setCursor(10, 20);
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.print("IP: ");
  tft.println(WiFi.localIP());
  
  // ========== LED Status ==========
  tft.setCursor(10, 45);
  tft.setTextSize(2);
  tft.setTextColor(led_state ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.printf("LED: %s\n", led_state ? "ON" : "OFF");
  
  // ========== Mode ==========
  tft.setCursor(10, 65);
  tft.setTextSize(1);
  tft.setTextColor(auto_mode ? TFT_GREEN : TFT_ORANGE, TFT_BLACK);
  tft.printf("Mode: %s\n", auto_mode ? "AUTO" : "MANUEL");
  
  // ========== Données Capteurs ==========
  tft.setCursor(10, 85);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.printf("Lum: %.1f%% (%d)\n", percent, adc_ldr);
  
  tft.setCursor(10, 100);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.printf("Temp: %.2f C\n", temperature);
  
  // ========== Seuils (si mode auto) ==========
  if (auto_mode) {
    tft.setCursor(10, 115);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    if (light_control_enabled) {
      tft.printf("Seuil Lum: %d\n", light_threshold);
    }
    if (temp_control_enabled) {
      tft.printf("Seuil Temp: %.1fC\n", temp_threshold);
    }
  }
}

//------------------------------------------------------
// Read all sensors
void readSensors() {
  adc_ldr = readLDR(LDR_PIN);
  temperature = readTemperature(THERM_PIN);
  // humidity reste à 0 pour compatibilité
}

//------------------------------------------------------
// Automatic control logic
void autoControl() {
  if (!auto_mode) return;
  
  bool should_turn_on = false;
  
  // Contrôle basé sur la lumière
  if (light_control_enabled && adc_ldr < light_threshold) {
    should_turn_on = true;
  }
  
  // Contrôle basé sur la température
  if (temp_control_enabled && temperature > temp_threshold) {
    should_turn_on = true;
  }
  
  // Mettre à jour l'état de la LED
  if (should_turn_on != led_state) {
    led_state = should_turn_on;
    digitalWrite(LED_PIN, led_state ? LOW : HIGH);
  }
}

//------------------------------------------------------
// Set LED state
void setLED(bool state) {
  led_state = state;
  digitalWrite(LED_PIN, led_state ? LOW : HIGH);
  updateDisplay();
}

//------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  // Configure pins
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED OFF (active-low)
  
  // Configure ADC
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  analogSetPinAttenuation(THERM_PIN, ADC_11db);
  
  // Initialize TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 30);
  tft.setTextSize(2);
  tft.println("Connecting...");
  
  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Initial sensor reading
  readSensors();
  updateDisplay();
  
  //==================== API ENDPOINTS ====================
  
  // Root endpoint
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "ESP32 IoT API v2.0");
  });
  
  // LED Control - ON
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    auto_mode = false;
    setLED(true);
    request->send(200, "application/json", 
      "{\"status\":\"success\",\"led\":\"ON\",\"mode\":\"manual\"}");
  });
  
  // LED Control - OFF
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    auto_mode = false;
    setLED(false);
    request->send(200, "application/json", 
      "{\"status\":\"success\",\"led\":\"OFF\",\"mode\":\"manual\"}");
  });
  
  // LED Control - TOGGLE
  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    auto_mode = false;
    setLED(!led_state);
    String response = led_state ? 
      "{\"status\":\"success\",\"led\":\"ON\",\"mode\":\"manual\"}" :
      "{\"status\":\"success\",\"led\":\"OFF\",\"mode\":\"manual\"}";
    request->send(200, "application/json", response);
  });
  
  // Get complete status (JSON)
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    StaticJsonDocument<512> doc;
    doc["led"] = led_state ? "ON" : "OFF";
    doc["mode"] = auto_mode ? "AUTO" : "MANUAL";
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["light"] = adc_ldr;
    doc["timestamp"] = millis();
    
    JsonObject thresholds = doc.createNestedObject("thresholds");
    thresholds["temp"] = temp_threshold;
    thresholds["temp_enabled"] = temp_control_enabled;
    thresholds["light"] = light_threshold;
    thresholds["light_enabled"] = light_control_enabled;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // Get sensor data only (JSON)
  server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
    StaticJsonDocument<256> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["light"] = adc_ldr;
    doc["light_percent"] = (adc_ldr / 4095.0) * 100.0;
    doc["timestamp"] = millis();
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // Legacy endpoints (text/plain pour compatibilité)
  server.on("/luminosity", HTTP_GET, [](AsyncWebServerRequest *request){
    float percent = (adc_ldr / 4095.0) * 100.0;
    String msg = "ADC = " + String(adc_ldr) + "\nLum = " + String(percent, 1) + "%\n";
    request->send(200, "text/plain", msg);
  });
  
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", 
      "Temperature = " + String(temperature, 2) + " C");
  });
  
// Auto mode control
server.on("/auto/on", HTTP_GET, [](AsyncWebServerRequest *request){
  auto_mode = true;
  updateDisplay(); // Déjà présent ✓
  request->send(200, "application/json", 
    "{\"status\":\"success\",\"mode\":\"AUTO\"}");
});

server.on("/auto/off", HTTP_GET, [](AsyncWebServerRequest *request){
  auto_mode = false;
  updateDisplay(); // Déjà présent ✓
  request->send(200, "application/json", 
    "{\"status\":\"success\",\"mode\":\"MANUAL\"}");
});
  
  // Set temperature threshold
// Set temperature threshold
server.on("/threshold/temp", HTTP_GET, [](AsyncWebServerRequest *request){
  if (request->hasParam("value")) {
    temp_threshold = request->getParam("value")->value().toFloat();
    
    if (request->hasParam("enabled")) {
      temp_control_enabled = request->getParam("enabled")->value() == "true";
    }
    
    // AJOUTEZ CETTE LIGNE ↓
    updateDisplay(); // Rafraîchir l'écran après modification
    
    StaticJsonDocument<128> doc;
    doc["status"] = "success";
    doc["temp_threshold"] = temp_threshold;
    doc["enabled"] = temp_control_enabled;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  } else {
    request->send(400, "application/json", 
      "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
  }
});
  
  // Set light threshold
// Set light threshold
server.on("/threshold/light", HTTP_GET, [](AsyncWebServerRequest *request){
  if (request->hasParam("value")) {
    light_threshold = request->getParam("value")->value().toInt();
    
    if (request->hasParam("enabled")) {
      light_control_enabled = request->getParam("enabled")->value() == "true";
    }
    
    // AJOUTEZ CETTE LIGNE ↓
    updateDisplay(); // Rafraîchir l'écran après modification
    
    StaticJsonDocument<128> doc;
    doc["status"] = "success";
    doc["light_threshold"] = light_threshold;
    doc["enabled"] = light_control_enabled;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  } else {
    request->send(400, "application/json", 
      "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
  }
});
  
  // Web control interface
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin: 20px; background: #f0f0f0; }
    .container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; }
    .button { padding: 15px 30px; margin: 10px; font-size: 18px; cursor: pointer; border: none; border-radius: 5px; }
    .on { background-color: #4CAF50; color: white; }
    .off { background-color: #f44336; color: white; }
    .auto { background-color: #2196F3; color: white; }
    .sensor { margin: 20px; padding: 15px; background: #e3f2fd; border-radius: 10px; }
    .sensor-value { font-size: 24px; font-weight: bold; color: #1976D2; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🌐 ESP32 IoT Control</h1>
    
    <div class="sensor">
      <h2>📊 Sensor Data</h2>
      <p>💡 Light: <span id="light" class="sensor-value">--</span> (<span id="lightPercent">--</span>%)</p>
      <p>🌡️ Temperature: <span id="temp" class="sensor-value">--</span> °C</p>
      <p>LED: <span id="ledStatus" class="sensor-value">--</span></p>
      <p>Mode: <span id="mode" class="sensor-value">--</span></p>
    </div>
    
    <h2>🎮 Manual Control</h2>
    <button class="button on" onclick="sendCommand('/on')">💡 LED ON</button>
    <button class="button off" onclick="sendCommand('/off')">🌑 LED OFF</button>
    <button class="button" onclick="sendCommand('/toggle')">🔄 Toggle</button>
    
    <h2>🤖 Auto Mode</h2>
    <button class="button auto" onclick="sendCommand('/auto/on')">✅ Enable Auto</button>
    <button class="button" onclick="sendCommand('/auto/off')">❌ Disable Auto</button>
  </div>
  
  <script>
    function sendCommand(url) {
      fetch(url).then(response => response.json()).then(data => {
        console.log(data);
        updateStatus();
      });
    }
    
    function updateStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          document.getElementById('temp').innerText = data.temperature.toFixed(2);
          document.getElementById('light').innerText = data.light;
          document.getElementById('lightPercent').innerText = ((data.light / 4095) * 100).toFixed(1);
          document.getElementById('ledStatus').innerText = data.led;
          document.getElementById('mode').innerText = data.mode;
        });
    }
    
    setInterval(updateStatus, 2000);
    updateStatus();
  </script>
</body>
</html>
    )rawliteral";
    request->send(200, "text/html", html);
  });
  
  server.begin();
  Serial.println("Server started");
  Serial.println("Available endpoints:");
  Serial.println("  GET /status - Complete status (JSON)");
  Serial.println("  GET /sensors - Sensor data (JSON)");
  Serial.println("  GET /on - Turn LED ON");
  Serial.println("  GET /off - Turn LED OFF");
  Serial.println("  GET /toggle - Toggle LED");
  Serial.println("  GET /auto/on - Enable auto mode");
  Serial.println("  GET /auto/off - Disable auto mode");
  Serial.println("  GET /threshold/temp?value=28&enabled=true");
  Serial.println("  GET /threshold/light?value=1000&enabled=true");
  Serial.println("  GET /control - Web interface");
  Serial.println("  GET /luminosity - Legacy text");
  Serial.println("  GET /temperature - Legacy text");
}

//------------------------------------------------------
void loop() {
  unsigned long currentMillis = millis();
  
  // Lire les capteurs périodiquement
  if (currentMillis - lastSensorRead >= sensorInterval) {
    lastSensorRead = currentMillis;
    readSensors();
    
    // Appliquer le contrôle automatique si activé
    autoControl();
    
    // Mettre à jour l'affichage
    updateDisplay();
  }
}