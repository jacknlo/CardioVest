# CardioCore V1 — ESP32-S3 Firmware

Firmware target for the **CardioCore V1** hardware module of the **CardioVest** project — an 8-channel, battery-powered ECG acquisition board.

> **Research / prototyping / education only.** This firmware is **NOT** a certified medical device and is not intended for diagnosis, treatment, patient monitoring, emergency use, or any clinical decision-making.

---

## 1. Overview

This firmware runs on the **ESP32-S3-WROOM-1-N16R8** (16 MB flash / 8 MB PSRAM) and is responsible for moving **raw** ECG samples off the board. It does the following:

1. **Initialize the analog front-end (AFE).** Configure the **TI ADS1298** (8-channel, 24-bit, simultaneous-sampling) over SPI: reset, set sample rate, configure channel gains, enable the right-leg drive (RLD) circuit, and select the **REF5025** external 2.5 V precision reference.
2. **Acquire samples on DRDY.** Service the ADS1298 `DRDY` (data-ready) interrupt and perform the SPI burst read of the status word plus all 8 channels per conversion frame.
3. **Buffer.** Push frames into a ring/FIFO buffer (PSRAM-backed) to decouple acquisition timing from the BLE and microSD consumers.
4. **Stream over BLE.** Publish buffered raw frames over a BLE GATT service for a host (PC / phone) to receive.
5. **Log to microSD.** Optionally persist raw frames to the microSD card for offline analysis.
6. **Report battery / charge status.** Read the 1S LiPo state (voltage / charge status) and surface it over BLE and/or local indicators.

The firmware's job is strictly **acquire → buffer → stream/log**. It does **not** filter for clinical purposes, classify, or interpret the ECG (see [Safety](#6-safety)).

### Signal / data path (high level)

```
 Electrodes (RA, LA, LL, RL/RLD, V1..V6)
        |
        v
   ADS1298 AFE  --(SPI + DRDY IRQ)-->  ESP32-S3
   (REF5025 ref)                          |
                                          +--> PSRAM ring buffer
                                          |        |
                                          |        +--> BLE GATT stream  --> host
                                          |        +--> microSD log (raw frames)
                                          +--> battery / charge status reporting
```

---

## 2. Toolchain

**Framework:** Arduino framework for ESP32-S3.

**Build environment:** **TBD** — final choice between the two options below is not yet decided:

- **PlatformIO** (recommended candidate) — reproducible builds, pinned framework/toolchain versions, scripted CI. Likely `platform = espressif32`, `board = esp32-s3-devkitc-1` (or a custom board JSON for CardioCore V1), `framework = arduino`.
- **Arduino IDE 2.x** — lower barrier to entry for quick bringup.

The final decision (and the exact framework/core version pin) will be recorded here once made.

### Expected dependencies (TBD until pinned)

- Arduino-ESP32 core (version: **TBD**)
- SPI (built-in)
- BLE stack: NimBLE-Arduino vs. built-in BLE — **TBD**
- SD / SD_MMC (microSD)
- ADS1298 access: custom driver in `src/` (no assumed third-party library)

> Board target, partition scheme (16 MB flash), and PSRAM (OPI, 8 MB) configuration flags are **TBD** and will be captured alongside the toolchain decision.

---

## 3. Folder layout

```
firmware/esp32_s3/
├── README.md            <- this file
├── src/                 <- firmware source (.cpp / .h): ADS1298 driver, BLE, SD, battery, main loop
└── notes/               <- engineering notes, decisions, scratch design docs
    └── firmware_plan.md <- detailed firmware architecture & task plan
```

- **`src/`** — all compilable firmware source. Keep the ADS1298 driver, BLE service, microSD logger, and battery monitor in separate translation units; the acquisition ISR and buffering live close to the main entry point.
- **`notes/`** — design rationale, register maps, pin assignments, open questions, and bringup observations. Not compiled.

---

## 4. Build & flash

> **Placeholder.** Concrete commands depend on the toolchain decision in [Section 2](#2-toolchain) and will be filled in once finalized.

### PlatformIO (candidate)

```bash
# Build
pio run

# Flash over USB-C
pio run --target upload

# Serial monitor
pio device monitor
```

### Arduino IDE (candidate)

1. Install the Arduino-ESP32 core (version **TBD**).
2. Select board: ESP32-S3 (exact variant **TBD**), enable PSRAM (OPI), set flash size to 16 MB, choose partition scheme **TBD**.
3. Connect CardioCore V1 over USB-C, select the serial port, and upload.

> Exact board settings, partition table, upload speed, and any DFU/boot-button procedure are **TBD** pending hardware bringup.

---

## 5. Status

Early scaffolding. No verified build yet. Pin assignments, register configuration values, sample rates, and buffer sizes are **TBD** and will be confirmed during hardware bringup.

---

## 6. Safety

- This firmware and the CardioCore V1 hardware are for **research, prototyping, and education only**. They are **NOT** a certified medical device.
- The firmware **only moves, buffers, streams, and logs raw ECG samples**. It performs **no clinical interpretation**: no diagnosis, no arrhythmia/condition detection, no clinical-grade filtering, and no treatment, patient-monitoring, or emergency-use functionality.
- Any downstream display or analysis of the data is the responsibility of the user and must **not** be used for clinical decision-making.
- No regulatory conformance (e.g., FDA / CE / ISO) is claimed or implied.
- Electrical safety: the board is intended to be operated from its **1S LiPo battery** for any human-subject experimentation. Follow appropriate isolation practices; do **not** connect electrodes to a person while the board is powered from mains-connected USB.

---

## 7. Related documents

- [`notes/firmware_plan.md`](notes/firmware_plan.md) — detailed firmware architecture, module breakdown, and task plan.
- [`docs/Bringup_Plan.md`](../../docs/Bringup_Plan.md) — hardware/firmware bringup sequence and verification steps.
