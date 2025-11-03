#include <TFT_eSPI.h>  
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

const int ledPin = 32;
const int buttonPin = 33;

bool led_state = false;
bool lastButtonState = HIGH; 
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
  tft.init();          
  tft.setRotation(1); 
  tft.fillScreen(TFT_RED);  
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); 


  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }

  Serial.println("Ready");
  updateDisplay();
}

void updateDisplay() {
  if (led_state) {
    tft.fillScreen(TFT_RED);
    tft.setCursor(10, 40);
    tft.setTextColor(TFT_BLACK, TFT_RED);
    tft.setTextSize(2);
    tft.println("LED is ON");
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 40);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.println("LED is OFF");
  }
}

void loop() {
  int reading = digitalRead(buttonPin);

  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
   
    if (reading == LOW && lastButtonState == HIGH) {
      led_state = !led_state;
      digitalWrite(ledPin, led_state ? HIGH : LOW);
      updateDisplay();
    }
  }

  lastButtonState = reading;
}
