const int LDR_PIN = 32;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
}

void loop() {
  int valeur = analogRead(LDR_PIN);
  Serial.print("Luminosité = ");
  Serial.println(valeur);
  delay(300);
}
