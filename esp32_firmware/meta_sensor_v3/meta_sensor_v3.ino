#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <IRremote.hpp>

#define IR_SEND_PIN 4
#define NUM_READINGS 5
#define VOTE_THRESHOLD 4

#define CMD_RED     11  
#define CMD_GREEN   22 
#define CMD_BLUE    37
#define CMD_BLACK   44
#define CMD_WHITE   51
#define CMD_YELLOW  58
#define CMD_CYAN    19
#define CMD_MAGENTA 62 

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_4X);

uint8_t history[NUM_READINGS] = {0};
int history_index = 0;

uint8_t last_sent = 0;
bool need_confirm_send = false;
unsigned long change_time_ms = 0;

uint8_t classifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
    if (c < 35) return CMD_BLACK;

    uint32_t sum = r + g + b;
    if (sum == 0) return CMD_BLACK;

    int rn = (r * 100) / sum;
    int gn = (g * 100) / sum;
    int bn = (b * 100) / sum;

    if (c > 260 && rn >= 28 && rn <= 40 && gn >= 28 && gn <= 40 && bn >= 28 && bn <= 40) {
        return CMD_WHITE;
    }
    
    if (c > 120 && rn >= 32 && gn >= 32 && bn < 24) return CMD_YELLOW;
    if (c > 120 && gn >= 30 && bn >= 30 && rn < 24) return CMD_CYAN;
    if (c > 120 && rn >= 21 && bn >= 35 && gn < 26) return CMD_MAGENTA;

    if (rn > 42 && rn > gn && rn > bn) return CMD_RED;
    if (gn > 42 && gn > rn && gn > bn) return CMD_GREEN;
    if (bn > 42 && bn > rn && bn > gn) return CMD_BLUE;

    return 0;
}

void setup() {
    Serial.begin(115200);
    IrSender.begin(IR_SEND_PIN);
    if (!tcs.begin()) {
        Serial.println("TCS34725 sensor error!");
        while (1);
    }
}

void loop() {
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);

    uint8_t current_reading = classifyColor(r, g, b, c);

    history[history_index] = current_reading;
    history_index = (history_index + 1) % NUM_READINGS;

    uint8_t candidate_command = 0;
    uint8_t max_count = 0;

    for (int i = 0; i < NUM_READINGS; i++) {
        uint8_t cmd = history[i];
        if (cmd == 0) continue; 
        
        uint8_t count = 0;
        for (int j = 0; j < NUM_READINGS; j++) {
            if (history[j] == cmd) count++;
        }
        if (count > max_count) {
            max_count = count;
            candidate_command = cmd;
        }
    }

    unsigned long now = millis();

    if (max_count >= VOTE_THRESHOLD && candidate_command != 0 && candidate_command != last_sent) {
        IrSender.sendRC5(0, candidate_command, 2);
        last_sent = candidate_command;
        change_time_ms = now;
        need_confirm_send = true;
    }

    if (need_confirm_send && (now - change_time_ms >= 80)) {
        IrSender.sendRC5(0, last_sent, 2);
        need_confirm_send = false;
    }

    delay(25);
}