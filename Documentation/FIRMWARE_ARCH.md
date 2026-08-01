# Firmware Architecture

## 1. Overview

The firmware runs on an Arduino Nano (ATmega328P) or ESP32 and is responsible for:

1. **Scanning:** Executing the two-stage scanning algorithm.
2. **Mux Control:** Selecting the active quadrant via digital pins.
3. **ADC Sampling:** Reading pen position voltage.
4. **Communication:** Sending coordinates to the host PC over serial (UART).
5. **Display Control:** Triggering LCD film erase when commanded.

## 2. Module Structure

```
Firmware/
├── src/
│   ├── main.ino          # Main loop: scan -> process -> transmit
│   ├── scanner.h/.cpp    # Two-stage scanning algorithm
│   ├── mux.h/.cpp        # Multiplexer control
│   └── protocol.h        # Serial packet format definitions
└── README.md             # Build and flash instructions
```

## 3. Scanning Loop

The main loop follows this state machine:

```
+--------------+
|   IDLE       | <- No pen detected (ADC < threshold)
+------+-------+
       | Pen detected (ADC > threshold)
       v
+--------------+
| GLOBAL_SCAN  | <- Read full-sheet X/Y -> determine quadrant
+------+-------+
       | Quadrant determined
       v
+--------------+
| DEEP_SCAN    | <- Mux selects quadrant, re-read with full ADC range
+------+-------+
       | Coordinates ready
       v
+--------------+
| TRANSMIT     | <- Send packet over serial
+------+-------+
       | Acknowledged / loop
       v
   (back to IDLE/GLOBAL)
```

## 4. Serial Protocol

### 4.1 Packet Format

Each coordinate packet is transmitted as a compact ASCII or binary frame:

**ASCII Format (human-readable):**
```
$X:0512,Y:0512,Q:1,P:1\n
```

**Binary Format (compact, 8 bytes):**
```
| Sync (1B) | X_H (1B) | X_L (1B) | Y_H (1B) | Y_L (1B) | Q (1B) | Pen (1B) | Checksum (1B) |
```

- **Sync byte:** 0xAA
- **X_H/X_L:** X coordinate (10-bit packed into 2 bytes)
- **Y_H/Y_L:** Y coordinate (10-bit packed into 2 bytes)
- **Q:** Quadrant index (0-3)
- **Pen:** 1 = pen down, 0 = pen up
- **Checksum:** XOR of all preceding bytes

### 4.2 Baud Rate

- Default: 115200 baud
- Packet rate: ~100-200 Hz (depending on scan cycle time)

## 5. Configuration Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `ADC_THRESHOLD` | 50 | Minimum ADC value to detect pen contact |
| `SAMPLES_PER_READ` | 4 | ADC oversampling for noise reduction |
| `BAUD_RATE` | 115200 | Serial baud rate |
| `DEBOUNCE_MS` | 2 | Debounce delay for pen detection |
| `MUX_PIN_A` | D6 | Mux control bit A |
| `MUX_PIN_B` | D7 | Mux control bit B |

## 6. Timing Budget

| Stage | Estimated Time |
|-------|----------------|
| Global X read (4 samples) | ~0.4 ms |
| Global Y read (4 samples) | ~0.4 ms |
| Mux switch + settle | ~0.1 ms |
| Deep scan X + Y (4 samples each) | ~0.8 ms |
| Serial transmit | ~0.1 ms |
| **Total cycle** | **~1.8 ms** (~550 Hz theoretical max) |

Target practical rate: **100-200 Hz** with overhead.
