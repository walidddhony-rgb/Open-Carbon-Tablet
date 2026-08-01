# Open-Source Interactive Carbon Tablet (OSICT)

<p align="center">
  <img src="Documentation/images/hero_banner.png" alt="OSICT Banner" width="600"/>
</p>

<p align="center">
  <a href="https://github.com/walidddhony-rgb/Open-Carbon-Tablet/blob/main/LICENSE.txt"><img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="License: MIT"/></a>
  <img src="https://img.shields.io/badge/version-0.2.0--alpha-orange.svg" alt="Version"/>
  <img src="https://img.shields.io/badge/platform-Arduino|ESP32-teal.svg" alt="Platform"/>
  <img src="https://img.shields.io/badge/cost-%3C%20$10-green.svg" alt="Cost"/>
  <img src="https://img.shields.io/badge/resolution-4M+%20points-red.svg" alt="Resolution"/>
</p>

## Project Overview

OSICT is a **flexible, ultra-low-cost, open-source** writing/drawing tablet that converts pen movement into digital signals using a carbon resistive film. It achieves precision of **over 4 million interactive points** using only a 10-bit ADC and inexpensive analog multiplexer chips, making it accessible for education, research, and biometric applications.

## Key Features

| Feature | Detail |
|---------|--------|
| Resolution | 4,194,304 interactive points (theoretical) |
| ADC Bit Depth | 10-bit (1024 levels/axis) |
| Scan Algorithm | Two-stage: Global + Localized Deep Scan |
| Cost | Under $10 total BOM |
| Display | Bistable LCD writing film (zero static power) |
| Biometric | Dynamic signature trajectory capture |
| Connectivity | USB Serial (115200 baud) |
| MCU Support | Arduino Nano, ESP32 |

## Applications

- **Remote Education:** Affordable digital writing tablet for online classrooms.
- **Biometric Verification:** Dynamic signature and handwriting authentication.
- **Flexible Electronics Research:** Open platform for resistive sensor development.
- **Digital Inclusion:** Low-cost alternative for underserved communities.

## Table of Contents

- [Scientific Principle](#scientific-principle)
- [Bill of Materials](#bill-of-materials-bom)
- [Repository Structure](#repository-structure)
- [Quick Start](#quick-start)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

## Scientific Principle

### 1. Carbon Resistive Film

A film of pure graphite acts as a **continuous electrical resistance** (resistive sheet). When a conductive pen applies pressure at point (x, y), it creates a local voltage divider, allowing position calculation.

### 2. Two-Stage Scanning Algorithm

| Stage | Description |
|-------|-------------|
| Global Scan | Apply voltage across the full sheet; read approximate (x, y) with 10-bit ADC |
| Virtual Segmentation | Divide the board into 4 quadrants programmatically |
| Localized Deep Scan | Re-apply voltage only across the identified quadrant; full 10-bit range maps to the quadrant, multiplying resolution |

**Effective resolution:** 4 x 1024 x 1024 = **4,194,304 points**

### 3. Analog Multiplexer (Mux)

A CD4052 dual 4:1 analog mux uses only 2 control bits to select 4 physical panels, reducing wiring and improving isolation.

### 4. Bistable LCD Display

A cholesteric LCD writing film displays strokes instantly with **zero power consumption**. The display is erased by a brief electrical pulse.

For the full mathematical model, see [Documentation/THEORY.md](Documentation/THEORY.md).

## Bill of Materials (BOM)

| Component | Qty | Approx. Cost |
|-----------|-----|--------------|
| Arduino Nano / ESP32 board | 1 | $3.00 - $5.00 |
| Mux chip (CD4052 / 74HC4051) | 1 | $0.50 |
| Carbon film (printed graphite) | 1 | $0.20 |
| Bistable LCD writing film | 1 | $2.00 - $4.00 |
| Jumper wires and resistors | - | $1.00 |
| Conductive pen (metal or carbon tip) | 1 | $0.50 |
| **Approximate Total** | | **< $10.00** |

> Full machine-readable BOM: [Hardware/BOM.csv](Hardware/BOM.csv)

## Repository Structure

```
Open-Carbon-Tablet/
├── .github/                    # GitHub templates and CI
│   └── ISSUE_TEMPLATE/         # Bug report and feature request templates
├── Documentation/              # Theory, design docs, and guides
│   ├── images/                 # Diagrams and photos
│   ├── THEORY.md               # Scientific principles and equations
│   ├── HARDWARE_DESIGN.md      # Circuit design and PCB notes
│   ├── FIRMWARE_ARCH.md        # Firmware architecture description
│   └── SOFTWARE_ARCH.md        # Python receiver architecture
├── Firmware/                   # Arduino/ESP32 firmware
│   ├── src/                    # Core source files
│   │   ├── main.ino            # Main entry point
│   │   ├── scanner.h/.cpp      # Two-stage scanning algorithm
│   │   ├── mux.h/.cpp          # Mux control interface
│   │   └── protocol.h          # Serial communication protocol
│   └── README.md               # Firmware build & flash instructions
├── Hardware/                   # Hardware design files
│   ├── schematics/             # Circuit schematics
│   │   └── schematic_notes.md  # Text-based schematic
│   ├── BOM.csv                 # Bill of materials (CSV)
│   └── README.md               # Hardware assembly guide
├── Software/                   # Host-side software
│   └── Python_Receiver/        # Python data receiver & visualizer
│       ├── receiver.py         # Serial data acquisition
│       ├── visualizer.py       # Real-time point plotting
│       ├── requirements.txt    # Python dependencies
│       └── README.md           # Software usage guide
├── ACKNOWLEDGMENTS.md          # Credits and references
├── CHANGELOG.md                # Version history
├── CONTRIBUTING.md             # Contribution guidelines
├── LICENSE.txt                 # MIT License
├── .gitignore                  # Git ignore rules
└── README.md                   # This file
```

## Quick Start

### Hardware Setup

1. Source all components from the [BOM](Hardware/BOM.csv).
2. Assemble the circuit per [Hardware/README.md](Hardware/README.md).
3. Flash firmware per [Firmware/README.md](Firmware/README.md).

### Software Setup

1. Install **Python 3.8+** and dependencies:
   ```bash
   cd Software/Python_Receiver/
   pip install -r requirements.txt
   ```

2. Run the real-time visualizer:
   ```bash
   python visualizer.py --port COM3 --baud 115200
   ```

3. Or run the console receiver with CSV export:
   ```bash
   python receiver.py --port COM3 --baud 115200 --export session.csv
   ```

## Documentation

| Document | Description |
|----------|-------------|
| [THEORY.md](Documentation/THEORY.md) | Scientific theory, resistive sheet model, and mathematical derivations |
| [HARDWARE_DESIGN.md](Documentation/HARDWARE_DESIGN.md) | Circuit architecture, PCB notes, and assembly guide |
| [FIRMWARE_ARCH.md](Documentation/FIRMWARE_ARCH.md) | Firmware state machine, serial protocol, and timing budget |
| [SOFTWARE_ARCH.md](Documentation/SOFTWARE_ARCH.md) | Python receiver design, data flow, and export formats |

## Contributing

We welcome contributions from developers, researchers, and hardware enthusiasts! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on:

- Bug reports and feature requests
- Firmware improvements
- Hardware design contributions
- Software enhancements
- Documentation improvements

## License

This project is licensed under the **MIT License** - see [LICENSE.txt](LICENSE.txt) for details.

## Contact

- **Repository:** [Open-Carbon-Tablet on GitHub](https://github.com/walidddhony-rgb/Open-Carbon-Tablet)
- **Maintainer:** [@walidddhony-rgb](https://github.com/walidddhony-rgb)
- **Issues:** [Open an issue](https://github.com/walidddhony-rgb/Open-Carbon-Tablet/issues)

## Citation

If you use OSICT in academic research, please cite:

```bibtex
@misc{osict2026,
  author       = {walidddhony-rgb},
  title        = {Open-Source Interactive Carbon Tablet (OSICT)},
  year         = {2026},
  publisher    = {GitHub},
  url          = {https://github.com/walidddhony-rgb/Open-Carbon-Tablet}
}
```

## Project Status

> **Alpha - Work in Progress**
> The project is under active development. Firmware, hardware schematics, and software components are being finalized and validated. We welcome community feedback and contributions.
