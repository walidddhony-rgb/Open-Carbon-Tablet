/*
 * OSICT - Open-Source Interactive Carbon Tablet
 * File: OSICT_Firmware_4Wires.ino
 * License: MIT
 * Copyright (c) 2026 walidddhony-rgb
 */

/**
 * Alternative Firmware for 4-Wire Version
 * Uses mathematical virtual segmentation (no Mux needed)
 * Lower accuracy but simpler hardware
 * 
 * Hardware:
 * - MCU: Arduino Nano
 * - Carbon Sheet: Single continuous sheet
 * - 4 Wires: Top, Bottom, Left, Right
 * - Pen: Conductive stylus connected to A0
 */

const int PIN_RIGHT  = 2;
const int PIN_LEFT   = 3;
const int PIN_TOP    = 4;
const int PIN_BOTTOM = 5;
const int PEN_PIN = A0;

const int MID_POINT = 512;
const int SAMPLES = 8;
const int DEBOUNCE_THRESHOLD = 15;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RIGHT, INPUT);
  pinMode(PIN_LEFT, INPUT);
  pinMode(PIN_TOP, INPUT);
  pinMode(PIN_BOTTOM, INPUT);
  Serial.println("OSICT v1.0 (4-Wire Edition)");
}

void loop() {
  int globalX = scanGlobalX();
  delayMicroseconds(20);
  int globalY = scanGlobalY();
  
  if (globalX < DEBOUNCE_THRESHOLD && globalY < DEBOUNCE_THRESHOLD) {
    return;
  }
  
  int segmentID = 0;
  int finalX, finalY;
  
  // Virtual segmentation (mathematical)
  if (globalX >= MID_POINT && globalY >= MID_POINT) {
    segmentID = 1;
    finalX = map(globalX, MID_POINT, 1023, 0, 1023) + 1024;
    finalY = map(globalY, MID_POINT, 1023, 0, 1023) + 1024;
  } 
  else if (globalX < MID_POINT && globalY >= MID_POINT) {
    segmentID = 2;
    finalX = map(globalX, 0, MID_POINT, 0, 1023);
    finalY = map(globalY, MID_POINT, 1023, 0, 1023) + 1024;
  } 
  else if (globalX >= MID_POINT && globalY < MID_POINT) {
    segmentID = 3;
    finalX = map(globalX, MID_POINT, 1023, 0, 1023) + 1024;
    finalY = map(globalY, 0, MID_POINT, 0, 1023);
  } 
  else {
    segmentID = 4;
    finalX = map(globalX, 0, MID_POINT, 0, 1023);
    finalY = map(globalY, 0, MID_POINT, 0, 1023);
  }
  
  Serial.print(segmentID);
  Serial.print(":");
  Serial.print(finalX);
  Serial.print(":");
  Serial.println(finalY);
  
  delay(10);
}

int scanGlobalX() {
  pinMode(PIN_RIGHT, OUTPUT);  digitalWrite(PIN_RIGHT, HIGH);
  pinMode(PIN_LEFT, OUTPUT);   digitalWrite(PIN_LEFT, LOW);
  pinMode(PIN_TOP, INPUT);     
  pinMode(PIN_BOTTOM, INPUT);  
  return filteredRead(PEN_PIN);
}

int scanGlobalY() {
  pinMode(PIN_TOP, OUTPUT);    digitalWrite(PIN_TOP, HIGH);
  pinMode(PIN_BOTTOM, OUTPUT); digitalWrite(PIN_BOTTOM, LOW);
  pinMode(PIN_RIGHT, INPUT);   
  pinMode(PIN_LEFT, INPUT);    
  return filteredRead(PEN_PIN);
}

int filteredRead(int pin) {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(10);
  }
  return sum / SAMPLES;
}
