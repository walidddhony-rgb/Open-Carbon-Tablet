#include "mux.h"

MuxController::MuxController(uint8_t pinA, uint8_t pinB)
  : _pinA(pinA), _pinB(pinB) {}

void MuxController::init() {
  pinMode(_pinA, OUTPUT);
  pinMode(_pinB, OUTPUT);
  selectChannel(0);
}

void MuxController::selectChannel(uint8_t channel) {
  digitalWrite(_pinA, channel & 0x01);
  digitalWrite(_pinB, (channel >> 1) & 0x01);
}
