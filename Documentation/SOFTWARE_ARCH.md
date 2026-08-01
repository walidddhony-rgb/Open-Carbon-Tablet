# Software Architecture (Python_Receiver)

## 1. Overview

The Python_Receiver software runs on the host PC and is responsible for:

1. **Serial Acquisition:** Receiving coordinate packets from the Arduino/ESP32 over USB.
2. **Parsing:** Decoding the binary or ASCII protocol into (x, y, quadrant, pen_state) tuples.
3. **Visualization:** Real-time plotting of pen trajectory.
4. **Data Export:** Saving stroke data to files (CSV, JSON) for biometric analysis.
5. **Future: Biometric Analysis:** Feature extraction for signature verification.

## 2. Module Structure

```
Software/Python_Receiver/
├── receiver.py           # Serial acquisition + packet parsing
├── visualizer.py          # Real-time trajectory plotting
├── requirements.txt       # Python dependencies
└── README.md              # Usage guide
```

## 3. Data Flow

```
[Arduino/ESP32] -> USB Serial -> [receiver.py] -> [Queue] -> [visualizer.py] -> Screen
                                    |
                                    +-> [CSV/JSON export]
```

## 4. Receiver Module (receiver.py)

### 4.1 Responsibilities

- Open serial port with configurable baud rate.
- Read bytes and parse packets using the protocol defined in protocol.h.
- Push parsed coordinates to a thread-safe queue.
- Handle connection errors and reconnection.

### 4.2 Key Classes

```
SerialReceiver
├── __init__(port, baud)
├── connect() -> bool
├── read_packet() -> Coordinate | None
├── run()  # Background thread
└── stop()
```

```
Coordinate (dataclass)
├── x: int        # 0-1023
├── y: int        # 0-1023
├── quadrant: int # 0-3
├── pen_down: bool
└── timestamp: float
```

## 5. Visualizer Module (visualizer.py)

### 5.1 Responsibilities

- Read coordinates from the queue.
- Render pen trajectory in real time on a canvas.
- Support pen-up/pen-down state (stroke segmentation).
- Provide UI controls: clear, save, erase display.

### 5.2 Visualization Options

- **Tkinter Canvas:** Lightweight, cross-platform, suitable for 2D trajectory.
- **PyVista (future):** For 3D pressure visualization or mesh-based rendering.

## 6. Data Export Format

### 6.1 CSV Format

```csv
timestamp,x,y,quadrant,pen_down
1711920000.123,512,512,1,1
1711920000.133,513,514,1,1
```

### 6.2 JSON Format

```json
{
  "session_id": "20260101_153000",
  "device": "OSICT_v0.1",
  "strokes": [
    {
      "stroke_id": 0,
      "points": [
        {"t": 1711920000.123, "x": 512, "y": 512, "q": 1},
        {"t": 1711920000.133, "x": 513, "y": 514, "q": 1}
      ]
    }
  ]
}
```

## 7. Future: Biometric Analysis Module

Planned features for signature verification:

- **Velocity profile:** Derive speed from timestamped points.
- **Acceleration profile:** Second derivative of position.
- **Stroke count:** Number of pen-up/pen-down transitions.
- **Total time:** Duration of signature.
- **Aspect ratio:** Bounding box dimensions.
- **DTW (Dynamic Time Warping):** For template matching of signatures.
