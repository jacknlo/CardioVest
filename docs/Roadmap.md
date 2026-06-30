# CardioVest — Project Roadmap

> **Scope & status:** CardioVest is a **research, prototyping, and education** platform for
> multi-channel ECG signal acquisition. It is **not** a certified medical device. Nothing in
> this roadmap implies or plans for diagnosis, treatment, patient monitoring, emergency use,
> or clinical decision-making. No regulatory conformance (FDA / CE / ISO) is claimed or
> planned. All ECG signals captured are for engineering characterization and research only.

This document tracks the ordered milestones for the CardioVest program, beginning with the
**CardioCore V1** hardware module (an 8-channel, battery-powered ECG acquisition board) and
extending toward the wearable **CardioVest** garment and higher channel counts.

**All dates are TBD.** Milestones are grouped into phases; later phases depend on the
completion (or partial completion) of earlier ones. Dependencies are noted per milestone.

---

## Phase 0 — Exploration & De-risking

The goal of Phase 0 is to validate the core analog-front-end concept and the toolchain at the
lowest possible cost and complexity, before committing to a custom board.

### M0. CardioCore Zero — minimal proof-of-concept
- **Target date:** TBD
- **Description:** Bring up the signal chain on off-the-shelf hardware: an **ADS1298 evaluation
  module** (or equivalent breakout) wired to an **ESP32-S3 dev kit**, with the **REF5025** 2.5 V
  reference either on the eval module or on a small adapter. Use a breadboard / jumper harness
  and a simple resistor-network or function-generator test signal (not a person) to confirm SPI
  communication, register configuration, continuous-read (DRDY) data flow, and basic noise floor.
- **Exit criteria:** Reliable 8-channel sample streaming from the AFE into the ESP32-S3 over SPI;
  raw counts logged to serial; measured input-referred noise documented as a baseline (value TBD).
- **Dependencies:** None (entry point of the program).

---

## Phase 1 — CardioCore V1 Hardware

Phase 1 turns the validated concept into a custom, battery-powered, integrated board.

### M1. CardioCore V1 schematic
- **Target date:** TBD
- **Description:** Capture the full schematic for CardioCore V1: ESP32-S3-WROOM-1-N16R8 (16 MB
  flash / 8 MB PSRAM), ADS1298 8-channel 24-bit simultaneous-sampling AFE, REF5025 external
  2.5 V precision reference, 1S LiPo battery management + USB-C charging, USB-C for programming,
  microSD interface, and the 10 electrode inputs (RA, LA, LL, RL/RLD right-leg drive, V1–V6).
  Define power domains, decoupling strategy, input protection, and analog/digital partitioning.
- **Exit criteria:** Reviewed schematic with all nets named, key component values selected (any
  genuinely undecided values marked **TBD**), and a draft bill of materials.
- **Dependencies:** M0 (lessons from the eval-module bring-up inform part selection and topology).

### M2. CardioCore V1 PCB layout
- **Target date:** TBD
- **Description:** Lay out the PCB with emphasis on low-noise analog design: clean reference and
  AFE supply routing, controlled ground partitioning, short/symmetric electrode traces, guarding
  where appropriate, antenna keep-out for the ESP32-S3 module, and mechanical placement of the
  USB-C connector, microSD slot, and battery connector. Target stack-up and board outline TBD.
- **Exit criteria:** DRC-clean layout, generated fabrication outputs (Gerbers / drill / pick-and-
  place), and a documented impedance/stack-up note.
- **Dependencies:** M1 (schematic frozen for the revision being laid out).

### M3. Prototype manufacturing
- **Target date:** TBD
- **Description:** Fabricate and assemble the first small run of CardioCore V1 boards (quantity
  TBD). Includes ordering bare boards, sourcing the BOM, assembly (vendor or in-house), and an
  incoming inspection / smoke-test checklist (power rails, reference voltage, USB enumeration).
- **Exit criteria:** At least one assembled board powering up with correct rail voltages and a
  living "known issues" / rework log for the revision.
- **Dependencies:** M2 (fabrication outputs), M1 (BOM).

---

## Phase 2 — Firmware, Tooling & First Capture

Phase 2 makes the hardware do something useful and gives engineers a way to see the data.

### M4. Firmware bring-up
- **Target date:** TBD
- **Description:** Bring up the ESP32-S3 firmware on real CardioCore V1 hardware: clocks and power,
  ADS1298 configuration and continuous-read pipeline, DMA/buffering for 8 simultaneous channels,
  BLE streaming service, microSD logging, and USB-C programming/serial. Implement a sane sample-
  rate configuration path (default rate TBD) and basic on-board housekeeping (battery voltage,
  status LEDs).
- **Exit criteria:** Board streams all 8 channels over BLE **and** logs to microSD without sample
  loss at the chosen rate; firmware builds reproducibly and is documented.
- **Dependencies:** M3 (working board), M0 (validated AFE driver logic).

### M5. Desktop viewer
- **Target date:** TBD
- **Description:** Build a cross-platform desktop application that connects to CardioCore V1 over
  BLE (and/or imports microSD log files), displays the 8 channels as live scrolling waveforms,
  shows basic per-channel statistics (offset, RMS noise, saturation flags), and exports raw data
  to an open format (e.g., CSV/EDF — final format TBD) for offline analysis. **Visualization and
  raw export only — no diagnostic or interpretive features.**
- **Exit criteria:** Stable live view of all 8 channels and a verified round-trip export/re-import
  of captured data.
- **Dependencies:** M4 (a stream/log to consume).

### M6. First ECG capture (research)
- **Target date:** TBD
- **Description:** Perform the first end-to-end research signal capture using the full chain
  (electrodes → CardioCore V1 → viewer/log), in a controlled engineering/research context with
  informed, consenting participants for the purpose of characterizing the platform. Focus is on
  signal-quality engineering metrics: noise, lead integrity, electrode-contact behavior, and
  cross-channel timing. **For research characterization only — not for any diagnostic or clinical
  purpose, and not relied upon for any health decision.**
- **Exit criteria:** A documented, reproducible capture session with raw data archived and signal-
  quality metrics recorded (target values TBD).
- **Dependencies:** M5 (viewer), M4 (firmware), and appropriate research/ethics review as required.

---

## Phase 3 — Wearable Integration (CardioVest)

Phase 3 moves from a bench board toward the wearable garment that gives the program its name.

### M7. CardioVest mechanical prototype (wearable garment integration)
- **Target date:** TBD
- **Description:** Integrate CardioCore V1 (or its next revision) into a wearable garment:
  electrode placement and retention within the vest, strain-relieved interconnect, board/battery
  enclosure and mounting, comfort and donning/doffing ergonomics, and basic mechanical/wash and
  cable-flex durability considerations. Materials and electrode-interface approach are TBD.
- **Exit criteria:** A wearable prototype that captures multi-channel signals comparable in quality
  to the bench setup, with a documented mechanical design and a list of integration issues.
- **Dependencies:** M6 (validated bench capture quality as the reference baseline), M3 (hardware).

---

## Phase 4 — Channel-Count Expansion

Phase 4 scales the platform beyond the initial 8 channels for higher-density research use.

### M8. Expansion to 16 / 32 / 64 channels
- **Target date:** TBD
- **Description:** Extend the architecture to higher channel counts by cascading/synchronizing
  multiple ADS1298 AFEs (the device supports daisy-chaining for simultaneous multi-device
  sampling). Address shared-reference distribution, inter-device sample synchronization, increased
  data bandwidth (streaming and storage), power budget, and the scaling of electrode harness and
  garment integration. Channel-count steps (16 → 32 → 64) are sequential sub-milestones; the exact
  AFE count, sample rates, and interconnect topology are TBD.
- **Exit criteria (per step):** Synchronized multi-AFE capture at the target channel count with
  documented inter-channel timing skew and aggregate throughput (targets TBD).
- **Dependencies:** M4 (single-board firmware/data pipeline as the foundation), M1–M3 (a hardware
  revision capable of hosting or interconnecting multiple AFEs), M7 (for wearable-scale harnessing).

---

## Dependency summary

```
M0 (CardioCore Zero)
   └─> M1 (Schematic)
          └─> M2 (PCB layout)
                 └─> M3 (Prototype manufacturing)
                        ├─> M4 (Firmware bring-up)   [also builds on M0]
                        │       ├─> M5 (Desktop viewer)
                        │       │       └─> M6 (First ECG capture, research)
                        │       │              └─> M7 (CardioVest mechanical prototype)
                        │       └─────────────────────> M8 (16/32/64-channel expansion)
                        └──────────────────────────────> (hardware base for M7, M8)
```

---

## Future ideas (Zio-inspired) — candidate enhancements

Inspired by how long-term ambulatory ECG patches (e.g., the Zio patch) operate.
**Research / engineering only — still no diagnosis or clinical interpretation.**

- **Continuous-logging model.** Record the raw stream continuously and never filter it
  destructively; treat noise handling as *classification/labeling*, not deletion, so the raw
  data is always preserved and nothing real is silently lost.
- **Symptom / event markers (button + voice).** Timestamp moments of interest on the ECG
  timeline — via a board button (SW1/SW2, firmware) and via **on-device voice annotations** in
  the mobile app (speech→text transcribed on the phone, text+timestamp saved, raw audio not
  retained). Markers share one track (`cardiovest.annotations.v1`, see `software/mobile_app/app_plan.md` §5.5).
- **Signal-quality index (SQI).** A processing step that scores each segment and labels it
  "good" vs "noise / uninterpretable" instead of forcing noisy data into a clean/abnormal
  bin. First brick of the DSP track (pairs with M5/M6).
- **Motion reference (accelerometer / IMU).** In a future board revision, add a small IMU
  (e.g., LSM6DS3) so motion can be correlated with signal disturbances to flag movement
  artifacts and provide activity context. **Not in the current BOM** — a future-revision idea.
- **Morphology-preserving filtering.** Keep filtering gentle (baseline-wander high-pass,
  50/60 Hz notch, conservative low-pass) so real waveform features are not distorted.

---

## Notes

- This roadmap is a living document; milestone ordering and scope may change as Phase 0/1
  findings come in. Genuinely undecided engineering values are marked **TBD** and should be
  resolved (not invented) as the work proceeds.
- Reminder of hard constraints: research/prototyping/education use only; no diagnostic,
  treatment, monitoring, or emergency-use functionality; no clinical interpretation of signals;
  no regulatory conformance claims.
