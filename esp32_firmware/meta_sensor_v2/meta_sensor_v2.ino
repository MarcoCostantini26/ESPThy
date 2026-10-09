#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <IRremote.hpp>

#define IR_SEND_PIN 4 

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

const int NUM_READINGS = 3;              
uint8_t reading_history[NUM_READINGS] = {4, 4, 4};
int history_index = 0;
uint8_t last_sent_command = 99;

void setup() {
  delay(2000);
  
  Serial.begin(115200);
  pinMode(DEBUG_LED, OUTPUT); 
  
  if (tcs.begin()) {
    Serial.println("RGB Meta-Sensor initialized (Fast Mode)!");
  } else {
    Serial.println("Sensor not found. Check connections.");
    while (1);
  }
  
  IrSender.begin(IR_SEND_PIN);
}

void loop() {
  uint16_t r, g, b, c;
  
  tcs.getRawData(&r, &g, &b, &c);
  
  uint8_t current_reading = 4;

  if (c < 150) { 
      current_reading = 4;
  } else if (r > 150 && g > 150 && b > 150) {
      current_reading = 5;
  } else if (r > g && r > b) {
      current_reading = 1;
  } else if (g > b && g > r) {
      current_reading = 2;
  } else if (b > g && b > r) {
      current_reading = 3;
  } else {
      current_reading = reading_history[history_index == 0 ? NUM_READINGS - 1 : history_index - 1]; 
  }

  reading_history[history_index] = current_reading;
  history_index = (history_index + 1) % NUM_READINGS;

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

  if (max_count >= 2 && candidate_command != last_sent_command) {
    IrSender.sendRC5(0, candidate_command, 2);
    last_sent_command = candidate_command;
    
    Serial.print("State change detected! RC5 command sent: ");
    Serial.println(candidate_command);

  }
}