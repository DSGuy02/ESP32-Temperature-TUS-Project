/*
 * Code written by Oladeji Sanyaolu 2025-2-27
 *
 * Processing Integration for ESP32 Temperature Project TUS
 *
*/

import processing.serial.*;

Serial myPort; // Create object from Serial class
String incomingData = ""; // Variable to store incoming serial data
float ds18b20Temp, dht11Temp, dht11Hum, lm35Temp; // Sensor values

void setup() {
  size(600, 400); // Window size
  println(Serial.list()); // Print available serial ports to the console
  
  // Open the serial port (change the port name based on your setup)
  String portName = "/dev/ttyACM0"; //Serial.list()[0]; // Change [0] to the correct port number if needed
  //myPort = new Serial(this, portName, 115200); // 115200 baud rate to match ESP32
  
  // Start with an empty string
  incomingData = "";
  
  // Set the frame rate for smooth updates (optional)
  frameRate(30);
}

void draw() {
  background(255); // White background
  
  // Display the sensor data
  fill(0);
  textSize(20);
  text("DS18B20 Temp: " + ds18b20Temp + " °C", 50, 50);
  text("DHT11 Temp: " + dht11Temp + " °C", 50, 100);
  //text("DHT11 Humidity: " + dht11Hum + " %", 50, 150);
  text("LM35 Temp: " + lm35Temp + " °C", 50, 200);
}

void serialEvent(Serial p) {
  // Read the incoming serial data
  incomingData = p.readStringUntil('\n'); // Read until newline character
  
  if (incomingData != null) {
    incomingData = trim(incomingData); // Remove leading and trailing spaces
    
    // Split the incoming CSV data by commas
    String[] data = split(incomingData, ',');
    
    if (data.length == 5) {
      // Parse the CSV data into sensor values
      try {
        ds18b20Temp = float(data[1]);  // DS18B20 temperature
        dht11Temp = float(data[2]);    // DHT11 temperature
        dht11Hum = float(data[3]);     // DHT11 humidity
        lm35Temp = float(data[4]);     // LM35 temperature
      } catch (Exception e) {
        println("Error parsing data: " + e.getMessage());
      }
    }
  }
}
