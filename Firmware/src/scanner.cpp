#include "scanner.h"

Scanner::Scanner(uint8_t adcPin, uint8_t driveXm, uint8_t driveXp,
                 uint8_t driveYm, uint8_t driveYp,
                 uint8_t samplesPerRead, uint16_t threshold)
  : _adcPin(adcPin), _driveXm(driveXm), _driveXp(driveXp),
    _driveYm(driveYm), _driveYp(driveYp),
    _samplesPerRead(samplesPerRead), _threshold(threshold) {}

void Scanner::init() {
  pinMode(_driveXm, OUTPUT);
  pinMode(_driveXp, OUTPUT);
  pinMode(_driveYm, OUTPUT);
  pinMode(_driveYp, OUTPUT);
  driveOff();
}

void Scanner::driveX() {
  digitalWrite(_driveXm, LOW);
  digitalWrite(_driveXp, HIGH);
  digitalWrite(_driveYm, LOW);
  digitalWrite(_driveYp, LOW);
  delayMicroseconds(50);
}

void Scanner::driveY() {
  digitalWrite(_driveYm, LOW);
  digitalWrite(_driveYp, HIGH);
  digitalWrite(_driveXm, LOW);
  digitalWrite(_driveXp, LOW);
  delayMicroseconds(50);
}

void Scanner::driveOff() {
  digitalWrite(_driveXm, LOW);
  digitalWrite(_driveXp, LOW);
  digitalWrite(_driveYm, LOW);
  digitalWrite(_driveYp, LOW);
}

uint16_t Scanner::readADC() {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < _samplesPerRead; i++) {
    sum += analogRead(_adcPin);
  }
  return (uint16_t)(sum / _samplesPerRead);
}

bool Scanner::isPenDown() {
  driveX();
  uint16_t val = readADC();
  driveOff();
  return val > _threshold;
}

Coordinate Scanner::readXY() {
  Coordinate coord = {0, 0, false};

  driveX();
  coord.x = readADC();

  driveY();
  coord.y = readADC();

  driveOff();

  coord.valid = (coord.x > _threshold || coord.y > _threshold);
  return coord;
}

Coordinate Scanner::globalScan() {
  return readXY();
}

uint8_t Scanner::determineQuadrant(const Coordinate& global) {
  bool right = (global.x >= 512);
  bool bottom = (global.y >= 512);

  if (!right && !bottom) return 0;
  if (right && !bottom)  return 1;
  if (!right && bottom)  return 2;
  return 3;
}

Coordinate Scanner::deepScan(uint8_t quadrant) {
  return readXY();
}

Coordinate Scanner::combineGlobal(const Coordinate& global,
                                  const Coordinate& deep,
                                  uint8_t quadrant) {
  Coordinate result;
  uint16_t halfX = 512;
  uint16_t halfY = 512;

  switch (quadrant) {
    case 0: result.x = deep.x / 2;          result.y = deep.y / 2;          break;
    case 1: result.x = halfX + deep.x / 2; result.y = deep.y / 2;          break;
    case 2: result.x = deep.x / 2;          result.y = halfY + deep.y / 2;  break;
    case 3: result.x = halfX + deep.x / 2; result.y = halfY + deep.y / 2;  break;
  }
  result.valid = true;
  return result;
}
