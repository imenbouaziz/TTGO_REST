#define CONFIG_ASYNC_TCP_USE_LWIP2 1
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define LED_PIN 32

const char* ssid = "Imim";
const char* password = "23232324";

AsyncWebServer server(80);
bool led_state = false;

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "API OK");
  });

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    led_state = true;
    digitalWrite(LED_PIN, HIGH);
    request->send(200, "text/plain", "LED ON");
  });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    led_state = false;
    digitalWrite(LED_PIN, LOW);
    request->send(200, "text/plain", "LED OFF");
  });

  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state ? HIGH : LOW);
    request->send(200, "text/plain", led_state ? "LED ON" : "LED OFF");
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", led_state ? "LED is ON" : "LED is OFF");
  });

  server.begin();
  Serial.println("Server started");
}

void loop() {}
