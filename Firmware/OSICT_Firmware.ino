/**
 * Project: Open-Source Interactive Carbon Tablet (OSICT)
 * File: OSICT_Firmware.ino
 * Description: High-resolution carbon-based resistive matrix with
 *              time-multiplexed localized scanning over 4 physical segments.
 * License: MIT
 *
 * Hardware:
 * - MCU: Arduino Nano (or any ATmega328P)
 * - Mux: CD4052 / 74HC4051 (2-bit selector)
 * - Carbon Sheet: 4 isolated segments
 * - Pen: Conductive stylus connected to A0
 */

// ==================== CONFIGURATION ====================

// Enable binary mode output in addition to ASCII
#define ENABLE_BINARY_MODE 1
// Protocol version for binary packets
#define PROTO_VERSION 1

// Mux Control Pins (2-bit selector)
const int MUX_S0 = 6;  // Bit 0 (LSB)
const int MUX_S1 = 7;  // Bit 1 (MSB)

// Shared Axis Pins (after Mux switching)
const int PIN_RIGHT  = 2;
const int PIN_LEFT   = 3;
const int PIN_TOP    = 4;
const int PIN_BOTTOM = 5;

// Pen input (analog)
const int PEN_PIN = A0;

// ==================== GLOBAL VARIABLES ====================

const int SAMPLES = 8;           // Number of samples for noise filtering
const int DEBOUNCE_THRESHOLD = 15; // Minimum reading to consider pen touching

// ==================== SETUP ====================

void setup() {
  Serial.begin(115200);
  
  // Configure Mux control pins
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  
  // Configure axis pins (initially as inputs)
  pinMode(PIN_RIGHT, INPUT);
  pinMode(PIN_LEFT, INPUT);
  pinMode(PIN_TOP, INPUT);
  pinMode(PIN_BOTTOM, INPUT);
  
  Serial.println("OSICT v1.0 Initialized.");
  Serial.println("SEG:X:Y");
}

// ==================== MAIN LOOP ====================

void loop() {
  // STEP 1: GLOBAL SCAN - Identify which segment the pen is touching
  int detectedSegment = -1;
  int maxReading = 0;
  
  for (int seg = 0; seg < 4; seg++) {
    selectSegment(seg);
    delayMicroseconds(50);
    
    // Quick X and Y scan to check if pen is on this segment
    int x = scanX();
    delayMicroseconds(20);
    int y = scanY();
    delayMicroseconds(20);
    
    // Combined reading to detect touch
    int reading = (x + y) / 2;
    
    if (reading > maxReading && reading > DEBOUNCE_THRESHOLD) {
      maxReading = reading;
      detectedSegment = seg;
    }
  }
  
  // STEP 2: If no segment detected, exit
  if (detectedSegment == -1) {
    return;
  }
  
  // STEP 3: LOCALIZED DEEP SCAN - Focus on the detected segment
  selectSegment(detectedSegment);
  delayMicroseconds(50);
  
  int localX = scanX();
  delayMicroseconds(20);
  int localY = scanY();
  
  // Pen flag: 1 = touching
  uint8_t penFlag = 1;
  
  // STEP 4: Send data over USB (ASCII)
  Serial.print(detectedSegment);
  Serial.print(":");
  Serial.print(localX);
  Serial.print(":");
  Serial.println(localY);

#if ENABLE_BINARY_MODE
  // Also send binary packet with protocol version and checksum
  sendBinaryPacket(detectedSegment, localX, localY, penFlag);
#endif
  
  delay(5); // 200 Hz polling rate
}

// ==================== MUX CONTROL ====================

/**
 * Select one of the 4 physical carbon segments using the Mux
 * @param segment: 0-3
 */
void selectSegment(int segment) {
  // Extract bits and send to Mux control pins
  digitalWrite(MUX_S0, (segment & 0x01));        // Bit 0 (LSB)
  digitalWrite(MUX_S1, ((segment >> 1) & 0x01)); // Bit 1 (MSB)
}

// ==================== AXIS SCANS ====================

/**
 * Scan X-axis (Left to Right)
 * @return: 0-1023 raw ADC value
 */
int scanX() {
  // Configure pins for horizontal measurement
  pinMode(PIN_RIGHT, OUTPUT);
  digitalWrite(PIN_RIGHT, HIGH); // 5V to Right
  pinMode(PIN_LEFT, OUTPUT);
  digitalWrite(PIN_LEFT, LOW);   // GND to Left
  pinMode(PIN_TOP, INPUT);       // Disable Top
  pinMode(PIN_BOTTOM, INPUT);    // Disable Bottom
  
  return filteredRead(PEN_PIN);
}

/**
 * Scan Y-axis (Top to Bottom)
 * @return: 0-1023 raw ADC value
 */
int scanY() {
  // Configure pins for vertical measurement
  pinMode(PIN_TOP, OUTPUT);
  digitalWrite(PIN_TOP, HIGH);   // 5V to Top
  pinMode(PIN_BOTTOM, OUTPUT);
  digitalWrite(PIN_BOTTOM, LOW); // GND to Bottom
  pinMode(PIN_RIGHT, INPUT);     // Disable Right
  pinMode(PIN_LEFT, INPUT);      // Disable Left
  
  return filteredRead(PEN_PIN);
}

// ==================== NOISE FILTERING ====================

/**
 * Read from analog pin with moving average filter
 * @param pin: analog pin to read
 * @return: filtered 0-1023 value
 */
int filteredRead(int pin) {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(10);
  }
  return sum / SAMPLES;
}

// ==================== BINARY PACKET SUPPORT ====================

/**
 * Binary packet format (extended):
 * [0xAA][PROTO_VER][X_H][X_L][Y_H][Y_L][SEG][PEN][CHECKSUM]
 * CHECKSUM = (PROTO_VER + X_H + X_L + Y_H + Y_L + SEG + PEN) & 0xFF
 */
void sendBinaryPacket(uint8_t segment, int x, int y, uint8_t pen) {
  uint8_t x_h = (x >> 8) & 0xFF;
  uint8_t x_l = x & 0xFF;
  uint8_t y_h = (y >> 8) & 0xFF;
  uint8_t y_l = y & 0xFF;
  uint8_t seg = segment & 0xFF;
  uint8_t proto = (uint8_t)PROTO_VERSION;

  uint16_t sum = 0;
  sum += proto;
  sum += x_h;
  sum += x_l;
  sum += y_h;
  sum += y_l;
  sum += seg;
  sum += pen;
  uint8_t checksum = sum & 0xFF;

  Serial.write(0xAA);
  Serial.write(proto);
  Serial.write(x_h);
  Serial.write(x_l);
  Serial.write(y_h);
  Serial.write(y_l);
  Serial.write(seg);
  Serial.write(pen);
  Serial.write(checksum);
}

// ==================== END OF CODE ====================
