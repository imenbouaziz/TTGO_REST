#include "api_routes.h"
#include "../sensors/light_sensor.h"
#include "../sensors/temp_sensor.h"
#include "../led/led_controller.h"

void registerRoutes(AsyncWebServer &server, int ldrPin, int tempPin, int ledPin) {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "API OK");
    });

    //luùm
    server.on("/luminosity", HTTP_GET, [ldrPin](AsyncWebServerRequest *request){
        int adc = readLDR(ldrPin);
        float voltage = calculateVoltage(adc);
        float percent = calculateLuminosityPercent(adc);

        String response = "ADC = " + String(adc) + "\n";
        response += "Voltage = " + String(voltage, 2) + " V\n";
        response += "Luminosity = " + String(percent, 1) + " %\n";
        request->send(200, "text/plain", response);
    });

    //temp
    server.on("/temperature", HTTP_GET, [tempPin](AsyncWebServerRequest *request){
        float tempC = readTemperature(tempPin);
        request->send(200, "text/plain", "Temperature = " + String(tempC, 2) + " C\n");
    });

    //led control
    server.on("/led", HTTP_POST, [ledPin](AsyncWebServerRequest *request){
        if (request->hasParam("state", true)) {
            String state = request->getParam("state", true)->value();
            setLEDState(ledPin, state == "on");
            request->send(200, "text/plain", "LED set to " + state);
        } else {
            request->send(400, "text/plain", "Missing 'state' parameter (utiliser 'on' ou 'off')");
        }
    });
}
