# Scientific Theory and Principles

## 1. Resistive Carbon Film Sensing

The core sensing mechanism relies on a **continuous resistive sheet** made of pure graphite. When a conductive pen applies pressure at point (x, y) on the film, it creates a local contact that divides the sheet into resistive regions. By measuring voltage drops across known boundaries, the pen position can be calculated.

### 1.1 Resistive Sheet Model

Consider a rectangular resistive sheet with uniform sheet resistance `R_s` (Ohm/square). When a voltage `V_in` is applied across the left and right edges, the voltage at any point (x, y) along the x-axis is:

```
V_x(x) = V_in * (x / W)
```

Where:
- `W` = width of the sheet
- `x` = horizontal position (0 to W)

Similarly, by applying voltage across top and bottom edges and measuring in the y-direction:

```
V_y(y) = V_in * (y / H)
```

Where:
- `H` = height of the sheet
- `y` = vertical position (0 to H)

### 1.2 Pen Contact as Voltage Divider

When the conductive pen touches the film, it acts as a **movable voltage probe**. The pen reads the local voltage at its contact point. By alternating between x-axis and y-axis voltage application (time-division multiplexing), both coordinates are obtained.

### 1.3 ADC Resolution and Theoretical Resolution

With a 10-bit ADC, the digital range is `0-1023` (1024 levels).

- **Linear resolution per axis:** `W / 1024` (or `H / 1024`)
- **Theoretical points:** `1024 x 1024 = 1,048,576` (~1M points)

## 2. Two-Stage Scanning Algorithm

To multiply the effective resolution without increasing ADC bit depth, a two-stage scanning approach is employed.

### 2.1 Stage 1: Global Scan

1. Apply voltage across the full sheet in x and y.
2. Read pen position with 10-bit ADC to get approximate (x, y).
3. Determine which **virtual quadrant** the pen is in (Q1, Q2, Q3, or Q4).

### 2.2 Stage 2: Virtual Segmentation and Localized Deep Scan

1. Divide the board into 4 quadrants programmatically.
2. Re-apply voltage **only across the identified quadrant** using Mux switching.
3. The full 10-bit ADC range now maps to the quadrant area instead of the full board.
4. Effective resolution per axis: `W/4 / 1024` which yields effective total points: `4 x 1024 x 1024 = 4,194,304 points`

### 2.3 Mathematical Model

Let the global position be `(x_g, y_g)` with quadrant index `q`:

```
x_global = x_quadrant_offset[q] + (ADC_x / 1023) * (W / 2)
y_global = y_quadrant_offset[q] + (ADC_y / 1023) * (H / 2)
```

Where `x_quadrant_offset` and `y_quadrant_offset` depend on the identified quadrant.

**Quadrant offset table:**

| Quadrant | x_offset | y_offset | Region |
|----------|----------|----------|--------|
| Q1 | 0 | 0 | Top-left |
| Q2 | W/2 | 0 | Top-right |
| Q3 | 0 | H/2 | Bottom-left |
| Q4 | W/2 | H/2 | Bottom-right |

## 3. Multiplexer (Mux) Role

A CD4052 (dual 4:1 analog mux) or 74HC4051 (8:1 analog mux) is used to:

1. **Select which physical panel/quadrant** to energize during deep scan.
2. **Route the pen signal** to the ADC input.
3. Minimize wiring (only 2 control bits needed for 4 channels).

### 3.1 Mux Control Logic (CD4052)

| Control Bit B | Control Bit A | Selected Channel |
|---------------|--------------|-------------------|
| 0 | 0 | Channel 0 (Q1) |
| 0 | 1 | Channel 1 (Q2) |
| 1 | 0 | Channel 2 (Q3) |
| 1 | 1 | Channel 3 (Q4) |

## 4. Bistable LCD Writing Film

The display layer uses a **bistable cholesteric LCD film** that:

- Reflects ambient light (no backlight needed).
- Retains the drawn image without power (zero static power consumption).
- Is erased by applying a brief pulse to clear the display.

### 4.1 Advantages

- Ultra-low power (power only needed during erase).
- Instant visual feedback for the user.
- Paper-like writing experience.

## 5. Biometric Signature Verification

The system captures not just the final signature image but the **dynamic writing trajectory**:

- **Temporal data:** Timestamps for each point to derive velocity and acceleration profiles.
- **Pressure proxy:** ADC value stability and contact quality to estimate pressure.
- **Trajectory data:** Ordered (x, y) points to determine stroke order and direction.

These features enable biometric analysis beyond simple image comparison.

## 6. References

- Carbon-based resistive touch sensor research.
- Arduino ADC resolution and voltage divider principles.
- CD4052 / 74HC4051 multiplexer datasheets.
- Bistable LCD film technology (e.g., Boogie Board).
