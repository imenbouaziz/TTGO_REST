const int thermistorPin = 32;  // ADC Pin connected to the NTC Thermistor
const float referenceVoltage = 3.3;  // Reference voltage (3.3V)
const int seriesResistor = 10000;  // Series resistor value (10kΩ)
const float beta = 3950;  // Beta value for the thermistor (depends on the thermistor used)
const float tempNominal = 25.0;  // Nominal temperature at 25°C
const int resistanceNominal = 10000;  // Nominal resistance at 25°C (10kΩ)

void setup() {
  Serial.begin(115200);
  pinMode(thermistorPin, INPUT);
}

void loop() {
  int adcValue = analogRead(thermistorPin);  // Read the ADC value (0-4095)
  float voltage = adcValue * (referenceVoltage / 4095.0);  // Convert ADC value to voltage
  float resistance = (seriesResistor * (referenceVoltage / voltage - 1));  // Calculate the resistance of the thermistor

  // Calculate temperature using the Steinhart-Hart equation or an approximation
  float temperature = 1.0 / (log(resistance / resistanceNominal) / beta + 1.0 / (tempNominal + 273.15)) - 273.15;

  // Print the temperature in Celsius
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  delay(1000);  // Wait for a second before taking another reading
}