#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Imim";
const char* password = "23232324";

AsyncWebServer server(80);


const int LED_PIN = 2;      
const int LIGHT_PIN = 34; 

int thresholdValue = 500;

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP = ");
  Serial.println(WiFi.localIP());

//test du endpint
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", "{\"msg\":\"hello world\"}");
  });

//----------------------------------
  server.on("/sensors/light", HTTP_GET, [](AsyncWebServerRequest *request){
    int val = analogRead(LIGHT_PIN);
    String json = "{\"light\":" + String(val) + "}";
    request->send(200, "application/json", json);
  });

  
  server.on("/led/on", HTTP_POST, [](AsyncWebServerRequest *request){
    digitalWrite(LED_PIN, HIGH);
    request->send(200, "text/plain", "LED ON");
  });

  server.on("/led/off", HTTP_POST, [](AsyncWebServerRequest *request){
    digitalWrite(LED_PIN, LOW);
    request->send(200, "text/plain", "LED OFF");
  });

  server.on("/threshold", HTTP_POST, [](AsyncWebServerRequest *request){
    if(request->hasParam("value", true)){
      thresholdValue = request->getParam("value", true)->value().toInt();
      request->send(200, "text/plain", "threshold updated");
    } else {
      request->send(400, "text/plain", "missing param: value");
    }
  });

  server.begin();
}

void loop() {
  int light = analogRead(LIGHT_PIN);

  if(light > thresholdValue)
    digitalWrite(LED_PIN, HIGH);
  else
    digitalWrite(LED_PIN, LOW);

  delay(50);
}
