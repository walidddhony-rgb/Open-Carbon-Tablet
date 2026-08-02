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

The firmware supports two serial output formats: a human-readable ASCII line and a compact binary frame. The Python receiver supports both and can be configured to accept only one format.

### 4.1 ASCII Format (human-readable)

A simple ASCII line is emitted for debugging and backwards compatibility:

```
SEG:X:Y\n
```

Examples:
```
0:512:512\n  # segment 0, x=512, y=512
```

This format is easy to inspect with a serial terminal but is larger on the wire.

### 4.2 Binary Formats

Two binary protocol versions exist in the firmware history. The receiver supports both versions; the firmware exposes a PROTO_VERSION macro that selects which payload layout is emitted.

- PROTO v1 (compact, legacy)
  - Payload after sync (0xAA): [PROTO][X_H][X_L][Y_H][Y_L][SEG][PEN][CHK]
  - Total after 0xAA = 8 bytes
  - X,Y: 16-bit values (10-bit ADC packed into 2 bytes)
  - CHK: checksum: sum of payload bytes (PROTO..PEN) & 0xFF (LSB)

  Example bytes (hex):
  0xAA 0x01 0x02 0x00 0x01 0x00 0x01 0x??

- PROTO v2 (extended: sequence + timestamp)
  - Payload after sync (0xAA):
    [PROTO (1)] [SEQ_H][SEQ_L] [TS_3][TS_2][TS_1][TS_0] [X_H][X_L] [Y_H][Y_L] [SEG] [PEN] [CHK]
  - Total after 0xAA = 14 bytes
  - SEQ: 16-bit packet sequence number (wraps at 0xFFFF)
  - TS: 32-bit timestamp in milliseconds (millis() after boot) big-endian
  - X,Y: 16-bit coordinates (high then low)
  - CHK: checksum = sum(payload bytes PROTO..PEN) & 0xFF (LSB)

  Example (pseudo-hex):
  0xAA 0x02 0x00 0x2A 0x00 0x00 0x01 0x5F 0x02 0x00 0x80 0x03 0x00 0x01 0x??

Notes on the checksum
- The checksum is a single byte equal to the LSB of the arithmetic sum of the payload bytes (PROTO through PEN). This is intentionally simple and fast to compute on MCU.
- The receiver should verify checksum and drop or log packets that fail verification.

### 4.3 Sync and Resynchronization
- Each binary packet begins with the sync byte 0xAA. The receiver should search for 0xAA to synchronize the stream and then read the expected number of payload bytes based on the PROTO value.
- If an unknown PROTO value is encountered, the receiver can attempt to resynchronize by skipping bytes until the next 0xAA or by reading until a newline for ASCII recovery.

### 4.4 Baud Rate and Packet Rate
- Default serial parameters: 115200 baud, 8N1.
- Packet rate: typically 100–200 Hz depending on scanning timing and MCU.

## 5. Runtime Configuration (firmware)

- `PROTO_VERSION` (macro): selects binary payload layout (1 or 2).
- `ENABLE_BINARY_MODE` (macro): enable/disable binary emission (if disabled, firmware only emits ASCII).
- You can add a runtime command on the serial control channel to toggle modes if desired.

## 6. Receiver behavior (host side)

- The Python receiver included in `Software/Python_Receiver/` supports both ASCII and binary frames.
- It implements:
  - automatic detection of 0xAA sync bytes;
  - parsing of PROTO v1 and v2 payloads;
  - checksum verification;
  - sequence tracking (v2) to detect dropped packets;
  - approximate latency calculation using the timestamp field (TS) in v2.

## 7. Configuration parameters (summary)

| Parameter | Default | Description |
|-----------|---------|-------------|
| `ADC_THRESHOLD` | 50 | Minimum ADC value to detect pen contact |
| `SAMPLES_PER_READ` | 4 | ADC oversampling for noise reduction |
| `BAUD_RATE` | 115200 | Serial baud rate |
| `ENABLE_BINARY_MODE` | 1 | Emit binary frames when enabled |
| `PROTO_VERSION` | 2 | Binary protocol version (1 or 2) |

## 8. Debugging tips

- If the receiver shows many checksum errors:
  - Verify the firmware and receiver agree on PROTO_VERSION.
  - Check wiring and USB serial stability (bad cables or power can corrupt bytes).
  - Lower scanning rate to reduce CPU/serial contention.

- If packet loss is high:
  - Increase baud rate if supported.
  - Reduce packet emission rate by disabling binary or ASCII as needed.


