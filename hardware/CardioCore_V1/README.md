# CardioCore V1

CardioCore V1 is the first hardware module of the **CardioVest** research platform: an 8-channel, battery-powered ECG acquisition board built around an ESP32-S3 microcontroller module and a Texas Instruments ADS1298 24-bit simultaneous-sampling analog front-end (AFE). It accepts the 10 standard electrode inputs (RA, LA, LL, RL/RLD, V1–V6), digitizes them with a shared REF5025 2.5 V precision reference, and streams or logs the raw samples over BLE and/or microSD. The board is powered by a 1S LiPo cell with USB-C charging and programming. CardioCore V1 is intended purely as a signal-acquisition front-end for experimentation; it performs **no** signal interpretation.

> **RESEARCH, PROTOTYPING, AND EDUCATION USE ONLY.**
> CardioCore V1 is **not** a medical device. It is **not** certified, cleared, or approved by any regulatory body (no FDA / CE / ISO conformance is claimed or implied). It must **not** be used for diagnosis, treatment, patient monitoring, emergency use, or any clinical decision-making. See [`../../docs/Safety_Research_Use.md`](../../docs/Safety_Research_Use.md) before building or operating this hardware.

## Key Components

- **MCU module — ESP32-S3-WROOM-1-N16R8:** Dual-core Xtensa LX7 with Wi-Fi/BLE, 16 MB flash and 8 MB PSRAM; hosts firmware, BLE streaming, and microSD logging.
- **ECG analog front-end — TI ADS1298:** 8-channel, 24-bit, simultaneous-sampling delta-sigma AFE with integrated PGAs, internal right-leg-drive (RLD) amplifier, and lead-off detection; interfaces to the ESP32-S3 over SPI.
- **Voltage reference — TI REF5025:** External 2.5 V precision, low-drift reference supplying the ADS1298 to improve measurement stability over the internal reference.
- **LiPo charger / power-path:** Single-cell (1S) Li-ion/LiPo charger with USB-C input and load-sharing power path so the board can run while charging. Specific charger IC — TBD.
- **3.3 V regulator:** Low-noise LDO supplying the digital and analog rails for the ESP32-S3, ADS1298, and peripherals. Exact part and analog/digital rail split — TBD.
- **microSD:** SPI- or SDMMC-attached card slot for local raw-sample logging. Interface mode — TBD.
- **USB-C:** Single USB-C connector for battery charging and firmware programming / serial. Native-USB vs. UART-bridge programming path — TBD.
- **ECG electrode connector:** Input connector for the 10 electrodes (RA, LA, LL, RL/RLD, V1–V6). Connector type and pinout — TBD.
- **ESD / input protection:** Per-channel input protection network (series resistance, ESD diodes/TVS, RF/EMI filtering) on every electrode line ahead of the AFE. Component selection — TBD.

## Current Status

**Design phase.** This module is in early design and has no released deliverables yet.

| Item | Status |
| --- | --- |
| Architecture concept | Drafted (see architecture doc below) |
| Schematic | TBD — not yet captured |
| PCB layout | None yet |
| Bill of materials | Preliminary / incomplete |
| Manufacturing files | None yet |

Where a real engineering value is still open, it is marked **TBD** rather than guessed. Do not treat any value in this repository as final until the schematic is captured and reviewed.

## Repository Layout

This module's deliverables are organized into the following subfolders:

- [`schematics/`](./schematics/) — Schematic source and exports (circuit design, net connectivity, design notes). Currently TBD.
- [`pcb/`](./pcb/) — PCB layout source, board stackup, and fabrication outputs (Gerbers, drill files). Currently empty; no PCB exists yet.
- [`bom/`](./bom/) — Bill of materials: component selections, manufacturer part numbers, and sourcing notes. Preliminary only.
- [`manufacturing/`](./manufacturing/) — Assembly and fabrication packages: pick-and-place files, assembly drawings, panelization, and fab-house notes. Generated once a PCB exists.

## Further Reading

- **Architecture:** [`../../docs/CardioCore_Architecture_v1.md`](../../docs/CardioCore_Architecture_v1.md) — block diagram, signal chain, power tree, and interface definitions for CardioCore V1.
- **Safety & research-use policy:** [`../../docs/Safety_Research_Use.md`](../../docs/Safety_Research_Use.md) — required reading on the research-only scope, electrical safety, and isolation considerations.

---

*Reminder: CardioVest and CardioCore V1 are for research, prototyping, and education only. This is not a medical device and must not be used for diagnosis, treatment, monitoring, emergency use, or clinical decision-making.*
