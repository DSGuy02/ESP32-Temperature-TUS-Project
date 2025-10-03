// ESP32 + DS18B20 + DHT11 + LM35 with BME280 and Blynk IoT and Serial output (CSV + JSON)
// Arduino IDE: install libraries via Library Manager:
// - Blynk (by Volodymyr Shymanskyy) - use Blynk IoT
// - OneWire (by Jim Studt et al.)
// - DallasTemperature (by Miles Burton)
// - DHT sensor library (by Adafruit)
// - Adafruit BME280 library (by Adafruit) // Add this for BME280 support

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

// WiFi credentials
char ssid[] = "D.S CE 3";
char pass[] = "01ad3j!5";

// Blynk credentials
#define BLYNK_TEMPLATE_ID "TMPL41g5tJELF"
#define BLYNK_TEMPLATE_NAME "ESP32 Temperature Project"
#define BLYNK_AUTH_TOKEN "FT8NQivsEHiGXQz8zD0asjN_ROqkSgdC"

// Pins
const int DS18B20_PIN = 4; // OneWire pin for DS18B20 (GPIO 4)
const int DHT_PIN = 5; // Digital pin for DHT11 (GPIO 5)
const int LM35_PIN = 34; // Analog input for LM35 (GPIO 34)

// Sensors
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

// BME280 sensor
Adafruit_BME280 bme; // I2C

// Blynk Virtual Pins
#define VPIN_DS18B20 V0
#define VPIN_DHT_TEMP V1
#define VPIN_LM35 V2
#define VPIN_BME280_TEMP V3

BlynkTimer timer;

// ADC/LM35 settings
const float ADC_VREF = 3.3f;
const int ADC_RES = 4095;
const int LM35_SAMPLES = 10;

float LM35_OFFSET_C = 0.0f; // Optional offset if needed

bool validFloat(float v) {
  return !isnan(v) && isfinite(v);
}

float readLM35C() {
  uint32_t sum = 0;
  for (int i = 0; i < LM35_SAMPLES; i++) {
    sum += analogRead(LM35_PIN);
    delay(2);
  }
  float avg = sum / (float)LM35_SAMPLES;
  float volts = (avg * ADC_VREF) / ADC_RES;
  float tempC = volts * 100.0f + LM35_OFFSET_C;
  return tempC;
}

float readDHTTempC() {
  float t = dht.readTemperature();
  if (!validFloat(t)) return NAN;
  return t;
}

float readDS18B20C() {
  ds18b20.requestTemperatures();
  float t = ds18b20.getTempCByIndex(0);
  if (t == 85.0f || t <= -127.0f) return NAN;
  return t;
}

float readBME280TempC() {
  float t = bme.readTemperature();
  if (!validFloat(t)) return NAN;
  return t;
}

void sendSensorData() {
  float t_ds = readDS18B20C();
  float t_dht = readDHTTempC();
  float t_lm35 = readLM35C();
  float t_bme = readBME280TempC();

  // Print temperature values to the serial monitor
  Serial.print("DS18B20 Temp: ");
  Serial.print(validFloat(t_ds) ? String(t_ds, 2) : "NaN");
  Serial.print(" | DHT11 Temp: ");
  Serial.print(validFloat(t_dht) ? String(t_dht, 2) : "NaN");
  Serial.print(" | LM35 Temp: ");
  Serial.print(validFloat(t_lm35) ? String(t_lm35, 2) : "NaN");
  Serial.print(" | BME280 Temp: ");
  Serial.println(validFloat(t_bme) ? String(t_bme, 2) : "NaN");

  // Send data to Blynk
  if (validFloat(t_ds)) Blynk.virtualWrite(VPIN_DS18B20, t_ds);
  if (validFloat(t_dht)) Blynk.virtualWrite(VPIN_DHT_TEMP, t_dht);
  if (validFloat(t_lm35)) Blynk.virtualWrite(VPIN_LM35, t_lm35);
  if (validFloat(t_bme)) Blynk.virtualWrite(VPIN_BME280_TEMP, t_bme);
}

BLYNK_CONNECTED() {
  Blynk.syncAll();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // ADC config for LM35
  analogReadResolution(12);
  analogSetPinAttenuation(LM35_PIN, ADC_11db);

  // Start sensors
  dht.begin();
  ds18b20.begin();
  
  // Initialize BME280
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Connect to WiFi and Blynk
  Serial.println("Connecting to WiFi and Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Timer every 2 seconds
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}