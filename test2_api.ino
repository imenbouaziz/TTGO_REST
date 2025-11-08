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

  //les routes de controle LED
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    led_state = false;
    digitalWrite(LED_PIN, HIGH);
    request->send(200, "text/plain", "LED OFF");
  });

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    led_state = true;
    digitalWrite(LED_PIN, LOW);
    request->send(200, "text/plain", "LED ON");
  });

  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state ? LOW : HIGH);
    request->send(200, "text/plain", led_state ? "LED OFF" : "LED ON");
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", led_state ? "LED est ON" : "LED est OFF");
  });
 

  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", R"rawliteral(
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
    )rawliteral");
  });

  server.begin();
  Serial.println("Server started");
}

void loop() {}
