/*
 * OSICT - Open-Source Interactive Carbon Tablet
 * Main Firmware Entry Point
 *
 * Board: Arduino Nano (ATmega328P) / ESP32
 * Author: walidddhony-rgb
 * License: MIT
 */

#include "scanner.h"
#include "mux.h"
#include "protocol.h"

// --- Configuration ---
#define ADC_THRESHOLD    50
#define SAMPLES_PER_READ 4
#define BAUD_RATE        115200
#define DEBOUNCE_MS      2

// --- Pin Definitions ---
#ifdef ESP32
  #define PIN_ADC       34
  #define PIN_MUX_A     25
  #define PIN_MUX_B     26
  #define PIN_DRIVE_XM  27
  #define PIN_DRIVE_XP  14
  #define PIN_DRIVE_YM  12
  #define PIN_DRIVE_YP  13
  #define PIN_LCD_ERASE 15
#else
  #define PIN_ADC       A0
  #define PIN_MUX_A     6
  #define PIN_MUX_B     7
  #define PIN_DRIVE_XM  2
  #define PIN_DRIVE_XP  3
  #define PIN_DRIVE_YM  4
  #define PIN_DRIVE_YP  5
  #define PIN_LCD_ERASE 8
#endif

Scanner scanner(PIN_ADC, PIN_DRIVE_XM, PIN_DRIVE_XP,
                PIN_DRIVE_YM, PIN_DRIVE_YP,
                SAMPLES_PER_READ, ADC_THRESHOLD);
MuxController mux(PIN_MUX_A, PIN_MUX_B);

unsigned long lastPenTime = 0;

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(PIN_LCD_ERASE, OUTPUT);
  digitalWrite(PIN_LCD_ERASE, LOW);
  mux.init();
  scanner.init();
  delay(100);
}

void loop() {
  if (scanner.isPenDown()) {
    Coordinate global = scanner.globalScan();

    if (global.valid) {
      uint8_t quadrant = scanner.determineQuadrant(global);

      mux.selectChannel(quadrant);
      delayMicroseconds(100);

      Coordinate deep = scanner.deepScan(quadrant);

      if (deep.valid) {
        Coordinate final = scanner.combineGlobal(global, deep, quadrant);

        Packet pkt;
        pkt.x = final.x;
        pkt.y = final.y;
        pkt.quadrant = quadrant;
        pkt.pen_down = 1;
        Protocol::sendPacket(&pkt);
      }
    }
    lastPenTime = millis();
  } else {
    if (millis() - lastPenTime > DEBOUNCE_MS) {
      Packet pkt;
      pkt.pen_down = 0;
      Protocol::sendPacket(&pkt);
    }
  }
}
