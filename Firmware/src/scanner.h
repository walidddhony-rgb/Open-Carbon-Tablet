/*
 * OSICT - Open-Source Interactive Carbon Tablet
 * Copyright (c) 2026 walidddhony-rgb
 * License: MIT (https://opensource.org/licenses/MIT)
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <Arduino.h>

struct Coordinate {
  uint16_t x;
  uint16_t y;
  bool valid;
};

class Scanner {
public:
  Scanner(uint8_t adcPin, uint8_t driveXm, uint8_t driveXp,
          uint8_t driveYm, uint8_t driveYp,
          uint8_t samplesPerRead, uint16_t threshold);

  void init();
  bool isPenDown();
  Coordinate globalScan();
  Coordinate deepScan(uint8_t quadrant);
  uint8_t determineQuadrant(const Coordinate& global);
  Coordinate combineGlobal(const Coordinate& global,
                           const Coordinate& deep,
                           uint8_t quadrant);

private:
  uint8_t _adcPin;
  uint8_t _driveXm, _driveXp, _driveYm, _driveYp;
  uint8_t _samplesPerRead;
  uint16_t _threshold;

  uint16_t readADC();
  void driveX();
  void driveY();
  void driveOff();
  Coordinate readXY();
};

#endif
