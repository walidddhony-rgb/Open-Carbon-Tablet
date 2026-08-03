



#ifndef TOUCH_DETECTOR_H
#define TOUCH_DETECTOR_H

#include <Arduino.h>

class TouchDetector {
public:
  TouchDetector(uint8_t pinOut, uint8_t pinIn);

  // يجري الـ handshake في كل مرة ويعيد true إذا وصل الكود عبر اللوح إلى القلم
  bool detectTouch();

private:
  uint8_t _pinOut;
  uint8_t _pinIn;

  static const uint8_t PATTERN_LEN = 9;
  // نمط بسيط 101101010 كما طلبت
  uint8_t _pattern[PATTERN_LEN] = {1,0,1,1,0,1,0,1,0};

  bool sendAndCheckPattern();
};

#endif
