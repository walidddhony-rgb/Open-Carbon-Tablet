# Hardware Design Document

## 1. System Overview

The OSICT hardware consists of three main layers stacked vertically:

```
+----------------------------------+
|  Layer 3: Bistable LCD Film      |  <- Visual display (top)
+----------------------------------+
|  Layer 2: Carbon Resistive Film  |  <- Sensing layer (middle)
+----------------------------------+
|  Layer 1: Rigid/Flexible Backing |  <- Structural support (bottom)
+----------------------------------+
```

## 2. Circuit Architecture

### 2.1 Microcontroller Interface

- **MCU:** Arduino Nano (ATmega328P) or ESP32
- **ADC:** 10-bit (Arduino) / 12-bit (ESP32)
- **Operating Voltage:** 5V (Arduino) / 3.3V (ESP32)

### 2.2 Resistive Sheet Wiring

The carbon film is wired with 4 edge electrodes:

| Electrode | Position | MCU Pin |
|-----------|----------|---------|
| Left (L)  | Left edge | D2 |
| Right (R) | Right edge | D3 |
| Top (T)   | Top edge | D4 |
| Bottom (B) | Bottom edge | D5 |

**X-axis measurement:** Drive L to R, read pen voltage on ADC.
**Y-axis measurement:** Drive T to B, read pen voltage on ADC.

### 2.3 Multiplexer Integration (CD4052)

The CD4052 dual 4:1 multiplexer is used to:

1. Select which quadrant's electrode pair to energize during deep scan.
2. Route the pen signal to the ADC pin (A0).

**Pin Connections:**

| CD4052 Pin | Connection |
|------------|------------|
| COM (X/Y)  | Arduino A0 (ADC) |
| X0         | Q1 pen signal |
| X1         | Q2 pen signal |
| X2         | Q3 pen signal |
| X3         | Q4 pen signal |
| A          | Arduino D6 |
| B          | Arduino D7 |
| VEE        | GND |
| VCC        | 5V |
| GND        | GND |

### 2.4 Power Supply

- **USB Power:** 5V via Arduino Nano USB.
- **LCD Film Erase:** Driven by a MOSFET (e.g., 2N7000) controlled by a digital pin.
- **Estimated total power:** < 50 mA during scanning.

## 3. PCB Design Notes

- Keep analog signal traces short to reduce noise.
- Add 100nF decoupling capacitor near the mux chip.
- Use a star ground configuration for analog and digital grounds.
- Pen input to ADC should have a series resistor (e.g., 1k Ohm) for protection.

## 4. Physical Construction

### 4.1 Layer Assembly

1. **Backing:** 3mm acrylic or PCB material cut to desired tablet size.
2. **Carbon Film:** Printed graphite film adhered to backing, with edge electrodes attached using conductive tape or silver epoxy.
3. **LCD Film:** Bistable LCD writing film placed on top of the carbon film, with the writable side facing up.
4. **Pen:** Metal or carbon-tipped conductive pen that presses through the LCD film to contact the carbon layer beneath.

### 4.2 Electrode Attachment

- Use copper tape along each edge of the carbon film.
- Solder wires to the copper tape edges.
- Ensure full edge coverage for uniform electric field distribution.

## 5. Safety Considerations

- Current limiting: The resistive sheet limits current naturally; add a 1k Ohm series resistor on the driving pins.
- ESD protection: Consider adding a TVS diode on the pen ADC input.
- Do not exceed the mux chip maximum ratings (VCC + 0.5V on analog pins).

## 6. Testing Points

| TP | Location | Expected Signal |
|----|----------|-----------------|
| TP1 | Left edge of carbon film | ~5V when driving X |
| TP2 | Top edge of carbon film | ~5V when driving Y |
| TP3 | CD4052 COM output | 0-5V variable (pen position) |
| TP4 | Mux control pin A (D6) | 0V or 5V (digital) |
| TP5 | Mux control pin B (D7) | 0V or 5V (digital) |
