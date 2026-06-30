# CardioCore V1

CardioCore V1 is the first hardware module of the **CardioVest** research platform: an 8-channel, battery-powered ECG acquisition board built around an ESP32-S3 microcontroller module and a Texas Instruments ADS1298 24-bit simultaneous-sampling analog front-end (AFE). It accepts the 10 standard electrode inputs (RA, LA, LL, RL/RLD, V1–V6), digitizes them with a shared REF5025 2.5 V precision reference, and streams or logs the raw samples over BLE and/or microSD. The board is powered by a 1S LiPo cell with USB-C charging and programming. CardioCore V1 is intended purely as a signal-acquisition front-end for experimentation; it performs **no** signal interpretation.

> **RESEARCH, PROTOTYPING, AND EDUCATION USE ONLY.**
> CardioCore V1 is **not** a medical device. It is **not** certified, cleared, or approved by any regulatory body (no FDA / CE / ISO conformance is claimed or implied). It must **not** be used for diagnosis, treatment, patient monitoring, emergency use, or any clinical decision-making. See [`../../docs/Safety_Research_Use.md`](../../docs/Safety_Research_Use.md) before building or operating this hardware.

## Key Components

- **MCU module — ESP32-S3-WROOM-1-N16R8:** Dual-core Xtensa LX7 with Wi-Fi/BLE, 16 MB flash and 8 MB PSRAM; hosts firmware, BLE streaming, and microSD logging.
- **ECG analog front-end — TI ADS1298:** 8-channel, 24-bit, simultaneous-sampling delta-sigma AFE with integrated PGAs, internal right-leg-drive (RLD) amplifier, and lead-off detection; interfaces to the ESP32-S3 over SPI.
- **Voltage reference — TI REF5025:** External 2.5 V precision, low-drift reference supplying the ADS1298 to improve measurement stability over the internal reference.
- **LiPo charger / power-path — MP2662 (MPS):** Single-cell (1S) LiPo charger with USB-C 5 V input and power-path so the board can run while charging. Charge current to be set conservatively for the wearable cell.
- **3.3 V regulator — AP2112K-3.3 (Diodes Inc.):** Low-noise 600 mA LDO feeding the 3.3 V rail (~340 mA active load plus ESP32 transient margin).
- **microSD — DS1139-06-08SS4BSR (CONNFLY):** Push-style microSD socket for local raw-sample logging over SPI.
- **USB-C — 16-pin receptacle (J1):** Sink-only USB-C for battery charging and native-USB programming/serial (D+/D− to the ESP32-S3). CC1/CC2 pull-downs (R1/R2, 5.1 kΩ) set the sink role.
- **ECG electrode connector — Molex 5034801000 (J3):** Input connector for the 10 electrodes (RA, LA, LL, RL/RLD, V1–V6), routed through ESD/protection/filtering to the ADS1298.
- **ESD / input protection — PESD3V3L5UY (D1/D2):** 5-channel low-capacitance ESD arrays at the electrode connector, ahead of the per-channel series resistors (R9–R20, **TBD**) and filter caps (C15–C30, **TBD**).
- **Expansion header — 2×20, 2.54 mm (J5):** Carries 3V3, GND, SPI, I2C, sync/control GPIO, and spare GPIO for stacking additional ECG-channel modules (the path toward 16/32/64 channels).

## Current Status

**Design phase.** This module is in early design and has no released deliverables yet.

| Item | Status |
| --- | --- |
| Architecture concept | Drafted (see architecture doc below) |
| Schematic | Captured in flux.ai (source `flux_project/`, EDIF netlist in `schematics/`) |
| PCB layout | In the flux.ai project; no fab outputs exported yet |
| Bill of materials | Derived from the flux.ai export — 64 components (some values TBD) |
| Manufacturing files | None released yet |

Where a real engineering value is still open, it is marked **TBD** rather than guessed. Do not treat any value in this repository as final until the schematic is captured and reviewed.

## Repository Layout

This module's deliverables are organized into the following subfolders:

- [`flux_project/`](./flux_project/) — **Editable design source** (`CardioCore_V1.flx`): the native flux.ai project (schematic + PCB).
- [`schematics/`](./schematics/) — Schematic exports and notes, including the EDIF netlist (`CardioCore_V1.edif`) exported from flux.ai.
- [`pcb/`](./pcb/) — PCB layout outputs (board stackup, Gerbers, drill files). The live layout currently lives in the flux.ai project; no fab outputs exported here yet.
- [`bom/`](./bom/) — Bill of materials: [`preliminary_bom.md`](./bom/preliminary_bom.md) (human-readable summary) plus [`flux_export/`](./bom/flux_export/) (the flux.ai CSV exports, per fab house).
- [`manufacturing/`](./manufacturing/) — Assembly and fabrication packages: pick-and-place files, assembly drawings, panelization, and fab-house notes. Generated once the PCB is finalized.

## Further Reading

- **Architecture:** [`../../docs/CardioCore_Architecture_v1.md`](../../docs/CardioCore_Architecture_v1.md) — block diagram, signal chain, power tree, and interface definitions for CardioCore V1.
- **Safety & research-use policy:** [`../../docs/Safety_Research_Use.md`](../../docs/Safety_Research_Use.md) — required reading on the research-only scope, electrical safety, and isolation considerations.

---

*Reminder: CardioVest and CardioCore V1 are for research, prototyping, and education only. This is not a medical device and must not be used for diagnosis, treatment, monitoring, emergency use, or clinical decision-making.*
