#define CONFIG_ASYNC_TCP_USE_LWIP2 1
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Imim";
const char* password = "23232324";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(2000);

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

  server.begin();
  Serial.println("Server started");
}

void loop() {
}
