#include "touch_detector.h"

TouchDetector::TouchDetector(uint8_t pinOut, uint8_t pinIn)
  : _pinOut(pinOut), _pinIn(pinIn) {}

bool TouchDetector::sendAndCheckPattern() {
  bool match = true;

  // 9 = OUTPUT لإرسال الكود، 11 = INPUT لقراءة القلم
  pinMode(_pinOut, OUTPUT);
  pinMode(_pinIn, INPUT);

  for (uint8_t i = 0; i < PATTERN_LEN; i++) {
    // إرسال البت على 9
    digitalWrite(_pinOut, _pattern[i] ? HIGH : LOW);
    delayMicroseconds(200);

    // قراءة 11
    int inVal = digitalRead(_pinIn);
    if (inVal != (_pattern[i] ? HIGH : LOW)) {
      match = false;
    }
  }

  // بعد انتهاء الـ handshake، نعيد 9 إلى وضع آمن قبل المسح
  digitalWrite(_pinOut, LOW);
  pinMode(_pinOut, INPUT);  // أو تركه منخفضًا إذا تفضل ذلك

  return match;
}

bool TouchDetector::detectTouch() {
  return sendAndCheckPattern();
}
