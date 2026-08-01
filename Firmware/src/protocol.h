#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

struct Packet {
  uint16_t x;
  uint16_t y;
  uint8_t  quadrant;
  uint8_t  pen_down;
};

class Protocol {
public:
  static void sendPacket(const Packet* pkt) {
    Serial.print("$X:");
    Serial.print(pkt->x);
    Serial.print(",Y:");
    Serial.print(pkt->y);
    Serial.print(",Q:");
    Serial.print(pkt->quadrant);
    Serial.print(",P:");
    Serial.println(pkt->pen_down);
  }

  static void sendBinaryPacket(const Packet* pkt) {
    uint8_t buffer[8];
    buffer[0] = 0xAA;
    buffer[1] = (pkt->x >> 8);
    buffer[2] = (pkt->x & 0xFF);
    buffer[3] = (pkt->y >> 8);
    buffer[4] = (pkt->y & 0xFF);
    buffer[5] = pkt->quadrant;
    buffer[6] = pkt->pen_down;
    buffer[7] = buffer[0] ^ buffer[1] ^ buffer[2] ^ buffer[3]
              ^ buffer[4] ^ buffer[5] ^ buffer[6];

    Serial.write(buffer, 8);
  }
};

#endif
