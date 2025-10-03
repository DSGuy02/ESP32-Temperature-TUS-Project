/*
 * Code written by Oladeji Sanyaolu 2025-2-27
 *
 * Processing Integration for ESP32 Temperature Project TUS
 *
*/

import processing.serial.*;

Serial myPort;
String incomingData = "";
float ds18b20Temp, dht11Temp, lm35Temp;

void setup() {
  size(600, 400);
  println(Serial.list());  // Print available serial ports
  
  String portName = "/dev/ttyACM0"; // Update this with your correct port
  myPort = new Serial(this, portName, 115200); // Match the baud rate with the ESP32
  
  incomingData = "";
  frameRate(30);
}

void draw() {
  background(255);
  
  fill(0);
  textSize(20);
  text("DS18B20 Temp: " + ds18b20Temp + " °C", 50, 50);
  text("DHT11 Temp: " + dht11Temp + " °C", 50, 100);
  text("LM35 Temp: " + lm35Temp + " °C", 50, 150);
}

void serialEvent(Serial p) {
  incomingData = p.readStringUntil('\n');
  
  if (incomingData != null) {
    incomingData = trim(incomingData);  // Remove spaces
    String[] data = split(incomingData, ',');
    
    if (data.length == 4) {
      try {
        ds18b20Temp = float(data[1]);  // DS18B20 temperature
        dht11Temp = float(data[2]);    // DHT11 temperature
        lm35Temp = float(data[3]);     // LM35 temperature
      } catch (Exception e) {
        println("Error parsing data: " + e.getMessage());
      }
    }
  }
}
