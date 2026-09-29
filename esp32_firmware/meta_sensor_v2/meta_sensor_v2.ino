#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <IRremote.hpp>

#define IR_SEND_PIN 4 

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// --- META-SENSOR & STATISTICAL FILTER VARIABLES ---
const int NUM_READINGS = 5;              
uint8_t reading_history[NUM_READINGS] = {4, 4, 4, 4, 4}; // Initialized to BLACK (4) for safety
int history_index = 0;
uint8_t last_sent_command = 99; // Anti-flooding memory

void setup() {
  Serial.begin(115200);
  
  if (tcs.begin()) {
    Serial.println("RGB Meta-Sensor initialized!");
  } else {
    Serial.println("Sensor not found. Check connections.");
    while (1);
  }
  
  IrSender.begin(IR_SEND_PIN);
  Serial.println("Statistical Filter (Majority Vote) and Anti-Flooding activated!");
}

void loop() {
  uint16_t r, g, b, c;
  
  // Hardware reading (takes 50ms)
  tcs.getRawData(&r, &g, &b, &c);
  
  uint8_t current_reading = 4; // Default: BLACK

  // STRICT CLASSIFICATION RULES FOR BACKLIT SCREENS
  if (c < 200) { 
      current_reading = 4; // BLACK
  } else if (c > 700) {
      current_reading = 5; // WHITE
  } else if (r > (g + 50) && r > (b + 50) && r > 120) {
      current_reading = 1; // RED
  } else if (g > (r + 50) && g > (b + 50) && g > 120) {
      current_reading = 2; // GREEN
  } else if (b > (r + 50) && b > (g + 50) && b > 120) {
      current_reading = 3; // BLUE
  } else {
      current_reading = 4; // Ambiguous reading/noise -> default to BLACK
  }

  // 1. UPDATE CIRCULAR BUFFER
  reading_history[history_index] = current_reading;
  history_index = (history_index + 1) % NUM_READINGS;

  // 2. CALCULATE STATISTICAL MODE (Majority Vote)
  uint8_t vote_counts[6] = {0};
  for (int i = 0; i < NUM_READINGS; i++) {
      if (reading_history[i] <= 5) vote_counts[reading_history[i]]++;
  }

  uint8_t candidate_command = 4;
  uint8_t max_count = 0;
  for (int i = 0; i <= 5; i++) {
      if (vote_counts[i] > max_count) {
          max_count = vote_counts[i];
          candidate_command = i;
      }
  }

  // 3. ANTI-FLOODING TRANSMISSION
  // Send command ONLY if it has a clear majority (>= 3 out of 5) 
  // AND is different from the last sent command.
  if (max_count >= 3 && candidate_command != last_sent_command) {
    IrSender.sendRC5(0, candidate_command, 2);
    last_sent_command = candidate_command;
    
    Serial.print("State change detected! RC5 command sent: ");
    Serial.println(candidate_command);
  }
}