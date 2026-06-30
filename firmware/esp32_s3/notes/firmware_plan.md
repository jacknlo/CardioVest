# CardioCore V1 — ESP32-S3 Firmware Development Plan

> **Scope and safety notice.** CardioCore V1 is a **research, prototyping, and education** platform. It is **not** a certified medical device. This firmware acquires, buffers, transports, and logs **raw electrophysiological samples only**. It performs **no** diagnosis, no clinical interpretation, no rhythm/morphology analysis, no treatment logic, no patient monitoring, and no emergency use. Any signal interpretation is the responsibility of downstream offline tooling operated by qualified researchers. Do not use this firmware for clinical decision-making.

---

## 1. Purpose

This document describes the planned architecture and development roadmap for the ESP32-S3 firmware that drives the CardioCore V1 8-channel ECG acquisition board. It is a living engineering document. Values that are not yet finalized are marked **TBD** and must not be treated as final design decisions.

### Target hardware (firmware-relevant facts)
- **MCU module:** ESP32-S3-WROOM-1-N16R8 (16 MB flash, 8 MB PSRAM, dual-core Xtensa LX7).
- **Analog front-end (AFE):** Texas Instruments ADS1298 — 8 channels, 24-bit, simultaneous sampling, SPI interface, `DRDY` data-ready interrupt line.
- **Reference:** REF5025 external 2.5 V precision reference feeding the ADS1298.
- **Electrode inputs (10):** RA, LA, LL, RL/RLD (right-leg drive), V1–V6.
- **Power:** 1S LiPo, USB-C for charging and programming.
- **Storage:** microSD (SD/SPI or SDMMC — interface choice **TBD**, see Open Questions).
- **Wireless:** BLE streaming.

---

## 2. Architecture Overview

The firmware uses **FreeRTOS** (built into ESP-IDF / Arduino-ESP32) to separate hard-real-time acquisition from softer-real-time transport and housekeeping. The ESP32-S3 is dual-core; the intent is to keep the acquisition path isolated from BLE/Wi-Fi stack jitter.

### 2.1 Task model

| Task | Priority (relative) | Pinned core (planned) | Trigger | Responsibility |
|------|--------------------|----------------------|---------|----------------|
| **Acquisition task** | Highest | Core 1 (APP_CPU) | `DRDY` GPIO interrupt | Read one full 8-channel frame from ADS1298 over SPI, timestamp it, push into ring buffer. |
| **BLE streaming task** | Medium | Core 0 (PRO_CPU, with BLE stack) | Ring-buffer data available | Drain frames, frame into BLE packets, notify subscribed central. |
| **SD logging task** | Medium-low | Core 1 or Core 0 (TBD) | Ring-buffer data available / block ready | Drain frames into a write buffer, flush blocks to microSD in raw binary. |
| **Power/status task** | Low | Either | Periodic timer (e.g. 1 Hz, TBD) | Read battery voltage / charger state, drive status LED(s), monitor lead-off and overflow flags, expose status over BLE. |

**Core-pinning rationale (planned, to be validated):** the BLE controller/host runs on PRO_CPU (Core 0). Pinning the acquisition task to APP_CPU (Core 1) reduces contention with the radio stack so `DRDY` servicing stays deterministic. Final pinning is **TBD** pending latency measurement (see milestones M5–M6).

### 2.2 Acquisition path (DRDY-driven)

```
ADS1298 DRDY (falling edge)
        │  GPIO ISR (minimal: notify acquisition task)
        ▼
Acquisition task
  1. Assert CS, SPI burst-read STATUS + 8 channels (3 bytes each)
  2. Capture monotonic timestamp (esp_timer / cycle count)
  3. Pack into a frame struct
  4. xRingbufferSend() / queue push  (non-blocking; drop-or-flag on full)
        ▼
   Ring buffer (lock-free / FreeRTOS ringbuf in PSRAM or internal RAM)
        ├──────────────► BLE streaming task (consumer)
        └──────────────► SD logging task   (consumer)
```

- The ISR itself does **no SPI** — it only signals the acquisition task (e.g. `vTaskNotifyGiveFromISR` / direct-to-task notification) to keep ISR latency low and avoid SPI from interrupt context.
- ADS1298 read is a **burst SPI transaction**: 1× STATUS word (24-bit) + 8× channel words (24-bit each) = 27 bytes per frame (continuous-read mode, `RDATAC`). Exact transaction layout depends on configured mode and is captured during bringup (M2).
- A single producer (acquisition) feeds **two consumers** (BLE + SD). Options under consideration: one shared ring buffer with two read cursors, or fan-out into two separate buffers. Choice is **TBD** (see Open Questions).

### 2.3 Ring buffer

- Decouples the hard-real-time producer from bursty/variable consumers (BLE congestion, SD write latency, SD card block-erase stalls).
- Candidate storage: large buffer in **PSRAM** (8 MB available) for deep logging headroom; small hot buffer in internal SRAM for lowest latency. Sizing is **TBD** and depends on chosen sample rate and worst-case consumer stall (see §3, Open Questions).
- **Overflow policy (TBD):** on full buffer, either drop newest frame, drop oldest frame, or stall. Whatever is chosen, overflow events **must be counted and surfaced** (status task / log header) so data gaps are never silent — research data integrity requires honest gap accounting.

---

## 3. Data Format Notes

The firmware treats samples as **opaque raw measurements**. No scaling to physical units, no filtering for interpretation, and no clinical labeling is applied on-device (light DSP such as decimation may be considered later but is out of scope for V1).

### 3.1 Sample frame (on the wire / on disk)
- **Per-channel sample:** 24-bit signed (two's complement), as delivered by the ADS1298. Stored/transmitted as 3 bytes per channel; MCU-side handling sign-extends to 32-bit only where arithmetic is needed.
- **Channels per frame:** 8 (fixed for CardioCore V1).
- **STATUS word:** the ADS1298 24-bit STATUS header (lead-off + GPIO bits) is captured per frame and **retained** — useful for research diagnostics; not interpreted clinically.
- **Sample rate:** **TBD.** ADS1298 supports a range (e.g. 250 SPS up to high-speed modes). Selection drives buffer sizing and BLE throughput budget; to be fixed during bringup. Whatever rate is chosen, it is recorded in the file/stream metadata.
- **Timestamp:** monotonic device timestamp per frame (source and resolution **TBD** — likely `esp_timer` microseconds) so downstream tools can detect dropped frames and reconstruct timing.

### 3.2 Packet framing (BLE and SD)
- **Packet framing:** **TBD.** Planned to include a sync/magic prefix, format/version byte, frame count, per-frame timestamp(s), and a payload of N frames, with an integrity check (CRC **TBD**).
- **BLE:** frames will be batched per notification to amortize per-packet overhead (batch size depends on negotiated MTU; see Open Questions). A versioned header lets the host parser stay forward-compatible.
- **SD:** raw binary, append-only, with a file header recording format version, channel count, configured sample rate, reference voltage (2.5 V via REF5025), ADS1298 register snapshot, and a session start timestamp. A companion sidecar metadata file (e.g. JSON) is under consideration.
- **Endianness and exact byte layout:** to be pinned down at M4/M5 and documented alongside the host-side parser so raw captures are unambiguous.

---

## 4. Milestones

Aligned with `docs/Bringup_Plan.md`. Each milestone is a verifiable firmware capability; do not advance until the prior one is demonstrated on hardware.

- **M0 — Toolchain & board bring-up.** Build/flash/serial confirmed on ESP32-S3-WROOM-1-N16R8. GPIO map for SPI, `DRDY`, `CS`, `START`, `PWDN/RESET`, SD, and status LEDs verified against the schematic. PSRAM enabled and detected.

- **M1 — SPI comms to ADS1298.** Establish SPI master link to the AFE. Verify clocking, CS timing, and basic command transactions (`RESET`, `SDATAC`/`RDATAC`, `WREG`/`RREG`). *Maps to Bringup: SPI comms.*

- **M2 — Register read / device ID.** Read the ADS1298 ID register and confirm the expected device signature; round-trip write/read of config registers. Confirms the AFE is alive and addressable. *Maps to Bringup: register read.*

- **M3 — Internal test signal.** Configure the ADS1298 internal test-signal generator, enable `DRDY`-driven acquisition, capture 8 channels via the ISR→acquisition-task→ring-buffer path, and verify the known test waveform over serial. Proves the full real-time acquisition pipeline without electrodes. *Maps to Bringup: internal test signal.*

- **M4 — SD logging.** Stand up the SD logging task; write raw framed binary with a session header to microSD at the target rate. Verify on a host that captured frames match the M3 test signal with no silent gaps (overflow counters checked). *Maps to Bringup: SD logging.*

- **M5 — BLE streaming.** Stand up the BLE streaming task and GATT service; stream framed test-signal data to a host app/parser. Measure sustained throughput and packet loss; tune batch size and MTU. *Maps to Bringup: BLE streaming.*

- **M6 — Battery-only acquisition.** Run the full system (acquisition + SD + BLE + power/status) on 1S LiPo with USB-C disconnected. Validate battery monitoring, charge/charge-state reporting, status LED behavior, and end-to-end stability for a defined session length (length **TBD**). *Maps to Bringup: battery-only acquisition.*

> **Out of scope for V1 (explicitly):** any on-device ECG interpretation, arrhythmia/rhythm detection, beat detection for clinical purposes, alarms, or diagnostic output. Real-electrode signal-quality work (lead-off handling via right-leg drive, etc.) is engineering validation only — never clinical readout.

---

## 5. Open Questions

These must be resolved (or explicitly deferred with rationale) before the design is frozen.

1. **Sample rate.** Which ADS1298 output data rate for V1? Trade-off: higher rate improves research fidelity but increases SPI load, buffer pressure, BLE bandwidth need, and power draw. *(Drives almost everything below.)* **TBD.**
2. **Ring-buffer sizing & placement.** Internal SRAM vs PSRAM split; depth in frames; worst-case consumer stall budget (SD block-erase latency, BLE congestion). Single shared buffer with two cursors vs fan-out to two buffers. **TBD.**
3. **Overflow policy.** Drop-newest vs drop-oldest vs stall, and how overflow/gap events are counted and surfaced to host and log header. **TBD.**
4. **BLE throughput.** Sustainable notification rate vs negotiated MTU, connection interval, and 2M PHY availability. Can BLE keep up with the chosen sample rate, or is BLE a downsampled/monitoring path while SD holds the full-rate capture? **TBD.**
5. **SD interface & sustained write rate.** SD-SPI vs SDMMC (1-bit/4-bit); filesystem (FATFS) vs raw block writes; sustained write throughput and flush strategy to avoid frame loss during card stalls. **TBD.**
6. **Power management.** Battery monitoring method (ADC divider vs fuel-gauge IC — depends on schematic), charge-state reporting, target session runtime, and any low-power/idle behavior. Acquisition determinism vs power saving trade-offs. **TBD.**
7. **Timestamp source & clock discipline.** Resolution and source; how host tools reconcile device time across BLE + SD captures. **TBD.**
8. **Packet/frame format & versioning.** Final byte layout, header fields, batching, and CRC scheme; must be co-designed with the host parser. **TBD.**
9. **Task core pinning & priorities.** Final pinning of acquisition vs BLE vs SD across the two cores, validated by measured `DRDY`-to-read latency and jitter. **TBD.**

---

## 6. Guiding Principles

- **Raw data only.** The firmware's job ends at honest acquisition, buffering, transport, and storage of raw 24-bit samples plus metadata. Interpretation lives off-device, with qualified researchers.
- **No silent data loss.** Every dropped/overflowed frame must be counted and reported. Research integrity depends on it.
- **Determinism first.** Protect the `DRDY` acquisition path from radio/storage jitter.
- **Safety-first framing.** No clinical, diagnostic, or regulatory claims anywhere in firmware, logs, BLE descriptors, or user-facing strings.
- **TBD means TBD.** Undecided engineering values stay marked until validated on hardware — no invented "final" numbers.
