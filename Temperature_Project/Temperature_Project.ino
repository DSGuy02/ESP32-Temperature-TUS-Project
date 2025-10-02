// ESP32 + DS18B20 + DHT11 + LM35 with Blynk IoT and Serial output (CSV + JSON)
// Arduino IDE: install libraries via Library Manager:
// - Blynk (by Volodymyr Shymanskyy) - use Blynk IoT
// - OneWire (by Jim Studt et al.)
// - DallasTemperature (by Miles Burton)
// - DHT sensor library (by Adafruit)
// Board: ESP32 Dev Module


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Set password to "" for open networks.

#define BLYNK_TEMPLATE_ID "TMPL41g5tJELF"
#define BLYNK_TEMPLATE_NAME "ESP32 Temperature Project"
#define BLYNK_AUTH_TOKEN "FT8NQivsEHiGXQz8zD0asjN_ROqkSgdC"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

// -------- WiFi credentials --------
char ssid[] = "D.S CE 3";
char pass[] = "01ad3j!5";

// -------- Pins (change to match your wiring) --------
const int DS18B20_PIN = 4;   // OneWire pin for DS18B20 (GPIO 4)
const int DHT_PIN     = 5;   // Digital pin for DHT11 (GPIO 5)
const int LM35_PIN    = 34;  // Analog input for LM35 (GPIO 34, input-only on ESP32)

// -------- Sensors --------
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

// -------- Blynk Virtual Pins --------
#define VPIN_DS18B20 V0
#define VPIN_DHT_TEMP V1
#define VPIN_DHT_HUM  V2
#define VPIN_LM35     V3

BlynkTimer timer;

// -------- ADC/LM35 settings --------
// ESP32 ADC: 12-bit (0..4095). LM35 outputs 10 mV/°C. Temp(°C) = Vout(V) * 100.
// Vout(V) = adc * (Vref/4095). Vref ~ 3.3V with 11dB attenuation.
const float ADC_VREF = 3.3f;
const int   ADC_RES  = 4095;
const int   LM35_SAMPLES = 10;

// Optional: calibration offset if needed (°C)
// e.g., measure a known temp and adjust.
float LM35_OFFSET_C = 0.0f;

// Utility: check if a float is valid
bool validFloat(float v) {
  return !isnan(v) && isfinite(v);
}

float readLM35C() {
  // Improve stability with small sample average
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
  float t = dht.readTemperature(); // Celsius
  if (!validFloat(t)) return NAN;
  return t;
}

float readDHTHumidity() {
  float h = dht.readHumidity();
  if (!validFloat(h)) return NAN;
  return h;
}

float readDS18B20C() {
  ds18b20.requestTemperatures();
  float t = ds18b20.getTempCByIndex(0);
  // Filter typical error codes
  if (t == 85.0f || t <= -127.0f) return NAN;
  return t;
}

void sendSensorData() {
  float t_ds   = readDS18B20C();
  float t_dht  = readDHTTempC();
  float h_dht  = readDHTHumidity();
  float t_lm35 = readLM35C();

  // Fallback strategy: if DHT returns NAN, keep it as NAN; dashboard/Processing can handle missing data.

  // ---- Serial output (human-readable) ----
  Serial.print("[Human] DS18B20_C: ");
  Serial.print(validFloat(t_ds) ? String(t_ds, 2) : "NaN");
  Serial.print(" | DHT11_C: ");
  Serial.print(validFloat(t_dht) ? String(t_dht, 2) : "NaN");
  Serial.print(" | DHT11_H: ");
  Serial.print(validFloat(h_dht) ? String(h_dht, 1) : "NaN");
  Serial.print("% | LM35_C: ");
  Serial.println(validFloat(t_lm35) ? String(t_lm35, 2) : "NaN");

  // ---- Serial output (CSV for Processing) ----
  // Header: timestamp_ms,ds18b20_c,dht11_c,dht11_h,lm35_c
  // Example line: 12345,24.50,24.00,45.0,24.70
  Serial.print(millis());
  Serial.print(",");
  Serial.print(validFloat(t_ds) ? String(t_ds, 2) : "NaN");
  Serial.print(",");
  Serial.print(validFloat(t_dht) ? String(t_dht, 2) : "NaN");
  Serial.print(",");
  Serial.print(validFloat(h_dht) ? String(h_dht, 1) : "NaN");
  Serial.print(",");
  Serial.println(validFloat(t_lm35) ? String(t_lm35, 2) : "NaN");

  // ---- Serial output (JSON for Processing) ----
  // Single-line JSON to simplify parsing
  Serial.print("{\"ts\":");
  Serial.print(millis());
  Serial.print(",\"ds18b20_c\":");
  Serial.print(validFloat(t_ds) ? String(t_ds, 2) : "null");
  Serial.print(",\"dht11_c\":");
  Serial.print(validFloat(t_dht) ? String(t_dht, 2) : "null");
  Serial.print(",\"dht11_h\":");
  Serial.print(validFloat(h_dht) ? String(h_dht, 1) : "null");
  Serial.print(",\"lm35_c\":");
  Serial.print(validFloat(t_lm35) ? String(t_lm35, 2) : "null");
  Serial.println("}");

  // ---- Blynk virtual writes ----
  if (validFloat(t_ds))   Blynk.virtualWrite(VPIN_DS18B20, t_ds);
  if (validFloat(t_dht))  Blynk.virtualWrite(VPIN_DHT_TEMP, t_dht);
  if (validFloat(h_dht))  Blynk.virtualWrite(VPIN_DHT_HUM,  h_dht);
  if (validFloat(t_lm35)) Blynk.virtualWrite(VPIN_LM35,     t_lm35);
}

BLYNK_CONNECTED() {
  // Optionally sync to fetch server-stored values if using input widgets
  Blynk.syncAll();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // ADC config for LM35
  analogReadResolution(12);
  analogSetPinAttenuation(LM35_PIN, ADC_11db); // ~0..3.3V range

  // Start sensors
  dht.begin();
  ds18b20.begin();

  // Print CSV header once for Processing parsing
  Serial.println("# CSV_HEADER: timestamp_ms,ds18b20_c,dht11_c,dht11_h,lm35_c");

  // Connect to Blynk
  Serial.println("Connecting to WiFi and Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Timer every 2 seconds (DHT11 minimum sample interval ~1s, so 2s is safe)
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}