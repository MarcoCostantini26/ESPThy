#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"

// --- Wi-Fi & UDP Settings ---
const char* ssid = "YOUR_WIFI_SSID";         // Replace with your network name
const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your network password
const char* host_ip = "192.168.1.100";       // Replace with the PC/Python server IP
const int udp_port = 5000;

WiFiUDP Udp;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() {
  Serial.begin(115200);

  // 1. Initialize Sensor
  if (tcs.begin()) {
    Serial.println("RGB Sensor initialized!");
  } else {
    Serial.println("Sensor not found. Check connections.");
    while (1);
  }

  // 2. Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWi-Fi connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  uint16_t r, g, b, c;
  
  // Read raw data from the sensor
  tcs.getRawData(&r, &g, &b, &c);
  
  String color = "Unknown";

  // Legacy Classification Rules (without majority vote)
  if (c < 80) { 
      color = "BLACK";
  } else if (r > 400 && g > 400 && b > 400) {
      color = "WHITE";
  } else if (r > g && r > b) {
      color = "RED";
  } else if (g > r && g > b) {
      color = "GREEN";
  } else if (b > r && b > g) {
      color = "BLUE";
  }

  // Send the color string via UDP if recognized
  if (color != "Unknown") {
    Udp.beginPacket(host_ip, udp_port);
    Udp.print(color);
    Udp.endPacket();
    
    Serial.print("UDP Packet sent: ");
    Serial.println(color);
  }
  
  delay(100); // Small delay to avoid flooding the network
}