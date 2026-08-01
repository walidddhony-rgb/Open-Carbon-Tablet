# Firmware - OSICT

This directory contains the firmware for the Open-Source Interactive Carbon Tablet.

## Hardware Requirements

- Arduino Nano (ATmega328P) or ESP32
- CD4052 or 74HC4051 multiplexer
- Carbon resistive film with edge electrodes
- Conductive pen

## Pin Mapping

| Function | Arduino Nano Pin | ESP32 Pin |
|----------|-----------------|-----------|
| ADC Pen Input | A0 | GPIO34 |
| Mux Control A | D6 | GPIO25 |
| Mux Control B | D7 | GPIO26 |
| Drive Left (X-) | D2 | GPIO27 |
| Drive Right (X+) | D3 | GPIO14 |
| Drive Top (Y-) | D4 | GPIO12 |
| Drive Bottom (Y+) | D5 | GPIO13 |
| LCD Erase | D8 | GPIO15 |

## Build and Flash

### Using Arduino IDE

1. Open `src/main.ino` in Arduino IDE.
2. Select board: `Arduino Nano` (or `ESP32 Dev Module`).
3. Select the correct COM port.
4. Click **Upload**.

### Using PlatformIO

```bash
cd Firmware/
pio run -t upload
```

## Configuration

Edit the `#define` constants at the top of `main.ino`:

```cpp
#define ADC_THRESHOLD    50    // Min ADC value to detect pen
#define SAMPLES_PER_READ 4     // Oversampling count
#define BAUD_RATE        115200
```

## Serial Output

Connect at 115200 baud. The firmware outputs coordinate packets:

**ASCII mode:**
```
$X:0512,Y:0512,Q:1,P:1
```

**Binary mode (8 bytes per packet):**
```
0xAA [X_H] [X_L] [Y_H] [Y_L] [Q] [PEN] [CHECKSUM]
```

## Testing

1. After flashing, open Serial Monitor at 115200 baud.
2. Touch the carbon film with the conductive pen.
3. You should see coordinate packets streaming.
4. Use the Python_Receiver to visualize the data.
