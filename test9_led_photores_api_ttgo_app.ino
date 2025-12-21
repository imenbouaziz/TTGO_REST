#define CONFIG_ASYNC_TCP_USE_LWIP2 1
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h>
#define LED_PIN 32
#define LDR_PIN 33
const char* ssid = "Imim";
const char* password = "23232324";
AsyncWebServer server(80);
TFT_eSPI tft = TFT_eSPI();
bool led_state = false;
int seuil = 1000; //seuil pour allumage auto(plus bas= plus sombre)
//-------------------------------------------------------
int readLDR(int pin, int samples = 10) {
uint32_t sum = 0;
for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
delay(5);
  }
return sum / samples;
}
void updateDisplay(int adcValue) {
float voltage = adcValue * 3.3 / 4095.0;
float percent = (adcValue / 4095.0) * 100.0;
tft.fillScreen(TFT_BLACK);
tft.setCursor(10, 30);
tft.setTextSize(2);
tft.setTextColor(led_state ? TFT_GREEN : TFT_RED, TFT_BLACK);
tft.printf("LED: %s\n", led_state ? "ON" : "OFF");
tft.setCursor(10, 70);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.printf("Lum: %.1f %%\n", percent);
tft.setCursor(10, 110);
tft.printf("ADC: %d\n", adcValue);
tft.setCursor(10, 150);
tft.printf("V: %.2f V\n", voltage);
tft.setCursor(10, 190);
tft.setTextSize(1);
tft.printf("IP: %s\n", WiFi.localIP().toString().c_str());
}
//-------------------------------------------------------
void setup() {
Serial.begin(115200);
delay(2000);
pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, HIGH); // LED OFF au démarrage (active-low)
analogReadResolution(12);
analogSetPinAttenuation(LDR_PIN, ADC_11db);
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
  //routes
server.on("/", HTTP_GET, <a href="AsyncWebServerRequest *request" target="_blank" rel="noopener noreferrer nofollow"></a>{
request->send(200, "text/plain", "API OK");
  });
server.on("/on", HTTP_GET, <a href="AsyncWebServerRequest *request" target="_blank" rel="noopener noreferrer nofollow"></a>{
    led_state = true;
digitalWrite(LED_PIN, LOW);
request->send(200, "text/plain", "LED ON");
  });
server.on("/off", HTTP_GET, <a href="AsyncWebServerRequest *request" target="_blank" rel="noopener noreferrer nofollow"></a>{
    led_state = false;
digitalWrite(LED_PIN, HIGH);
request->send(200, "text/plain", "LED OFF");
  });
server.on("/toggle", HTTP_GET, <a href="AsyncWebServerRequest *request" target="_blank" rel="noopener noreferrer nofollow"></a>{
    led_state = !led_state;
digitalWrite(LED_PIN, led_state ? LOW : HIGH);
request->send(200, "text/plain", led_state ? "LED ON" : "LED OFF");
  });
server.on("/status", HTTP_GET, <a href="AsyncWebServerRequest *request" target="_blank" rel="noopener noreferrer nofollow"></a>{
int adc = readLDR(LDR_PIN);
float percent = (adc / 4095.0) * 100.0;
    String response = "LED = " + String(led_state ? "ON" : "OFF") + "\n";
    response += "Luminosite = " + String(percent, 1) + " %\n";
request->send(200, "text/plain", response);
  });
  // Routes API pour Firebase
server.on("/api/status", HTTP_GET, <a href="AsyncWebServerRequest *request" target="_blank" rel="noopener noreferrer nofollow"></a>{
int adc = readLDR(LDR_PIN);
float percent = (adc / 4095.0) * 100.0;
  String json = "{";
  json += ""led_state":" + String(led_state ? "true" : "false") + ",";
  json += ""light_percentage":" + String(percent, 1) + ",";
  json += ""adc_value":" + String(adc) + ",";
  json += ""voltage":" + String(adc * 3.3 / 4095.0, 2) + ",";
  json += ""threshold":" + String(seuil);
  json += "}";
request->send(200, "application/json", json);
});
server.on("/api/control", HTTP_POST, <a href="AsyncWebServerRequest *request" target="_blank" rel="noopener noreferrer nofollow"></a>{
if(request->hasParam("command", true)) {
    String command = request->getParam("command", true)->value();
if(command == "on") {
      led_state = true;
digitalWrite(LED_PIN, LOW);
    } else if(command == "off") {
      led_state = false;
digitalWrite(LED_PIN, HIGH);
    } else if(command == "toggle") {
      led_state = !led_state;
digitalWrite(LED_PIN, led_state ? LOW : HIGH);
    }
request->send(200, "application/json", "{"success":true}");
  } else {
request->send(400, "application/json", "{"error":"Missing command"}");
  }
});
server.on("/api/settings", HTTP_POST, <a href="AsyncWebServerRequest *request" target="_blank" rel="noopener noreferrer nofollow"></a>{
if(request->hasParam("threshold", true)) {
    seuil = request->getParam("threshold", true)->value().toInt();
request->send(200, "application/json", "{"success":true}");
  } else {
request->send(400, "application/json", "{"error":"Missing threshold"}");
  }
});
  //------------------------------
server.on("/control", HTTP_GET, <a href="AsyncWebServerRequest *request" target="_blank" rel="noopener noreferrer nofollow"></a>{
int adc = readLDR(LDR_PIN);
float percent = (adc / 4095.0) * 100.0;
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <title>LED Control</title>
      </head>
      <body>
        <h2>Control LED</h2>
        <button onclick="fetch('/on')">Allumer</button>
        <button onclick="fetch('/off')">Eteindre</button>
        <button onclick="fetch('/toggle')">Basculer</button>
        <button onclick="checkStatus()">Etat</button>
        <p id="status"></p>
        <h3>Luminosité actuelle: )rawliteral";
    html += String(percent, 1) + " %</h3>";
    html += R"rawliteral(
        <script>
          function checkStatus() {
            fetch('/status')
              .then(response => response.text())
              .then(data => {
                document.getElementById('status').innerText = data;
              });
          }
        </script>
      </body>
      </html>
    )rawliteral";
request->send(200, "text/html", html);
  });
server.begin();
Serial.println("Server started");
}
//-------------------------------------------------------
void loop() {
int adc = readLDR(LDR_PIN);
  //adapter selon mon seuil (automatic turn on and off)
if (adc < seuil) {
    led_state = true;
digitalWrite(LED_PIN, LOW);
  } else {
    led_state = false;
digitalWrite(LED_PIN, HIGH);
  }
updateDisplay(adc);
delay(500);