#ifndef MUX_H
#define MUX_H

#include <Arduino.h>

class MuxController {
public:
  MuxController(uint8_t pinA, uint8_t pinB);
  void init();
  void selectChannel(uint8_t channel);

private:
  uint8_t _pinA;
  uint8_t _pinB;
};

#endif
