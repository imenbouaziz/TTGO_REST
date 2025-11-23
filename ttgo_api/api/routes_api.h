#pragma once
#include <ESPAsyncWebServer.h>

void registerRoutes(AsyncWebServer &server, int ldrPin, int tempPin, int ledPin);
