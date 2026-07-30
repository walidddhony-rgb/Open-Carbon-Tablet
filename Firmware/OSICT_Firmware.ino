/**
 * Project: Open-Source Interactive Carbon Tablet (OSICT)
 * File: OSICT_Firmware.ino
 * Description: High-resolution carbon-based resistive matrix with
 *              time-multiplexed localized scanning over 4 physical segments.
 * License: GNU GPLv3
 * 
 * Hardware:
 * - MCU: Arduino Nano (or any ATmega328P)
 * - Mux: CD4052 / 74HC4051 (2-bit selector)
 * - Carbon Sheet: 4 isolated segments
 * - Pen: Conductive stylus connected to A0
 */

// ==================== PIN DEFINITIONS ====================

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
  
  // STEP 4: Send data over USB
  Serial.print(detectedSegment);
  Serial.print(":");
  Serial.print(localX);
  Serial.print(":");
  Serial.println(localY);
  
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

// ==================== END OF CODE ====================