# CardioCore V1 — Architecture Document

**Project:** CardioVest — Wearable Multi-Channel ECG Acquisition (Research Platform)
**Module:** CardioCore V1 (8-channel acquisition board)
**Document status:** Draft v1 (living document; values marked **TBD** are not yet finalized)

---

## 1. Overview

CardioCore V1 is the first hardware module of the **CardioVest** research platform. It is a compact,
battery-powered, **8-channel, 24-bit simultaneous-sampling ECG acquisition board** built around three
core devices:

- **ESP32-S3-WROOM-1-N16R8** — the system controller (16 MB flash, 8 MB PSRAM), handling SPI
  acquisition, on-board buffering, BLE streaming, microSD logging, and power/charge supervision.
- **Texas Instruments ADS1298** — an 8-channel, 24-bit, simultaneous-sampling ECG analog front-end
  (AFE) with per-channel programmable gain, an integrated right-leg drive (RLD) amplifier, lead-off
  detection, and an internal test signal generator.
- **TI REF5025** — an external 2.5 V precision voltage reference feeding the ADS1298 to maximize
  measurement stability and reduce reference-induced noise.

The board acquires raw differential signals from up to **10 electrode inputs** (RA, LA, LL, RL/RLD,
V1–V6), digitizes them in the ADS1298, transfers samples over SPI to the ESP32-S3, and then makes the
raw data available over **BLE** and/or logs it to **microSD**. **USB-C** provides charging and
firmware programming. Power is sourced from a **1S LiPo** battery through a charger and power-path
front end into a 3.3 V regulated rail.

The intent of CardioCore V1 is to provide a clean, well-characterized **raw-signal acquisition
backbone** that later CardioVest modules (and higher channel counts) can build on.

---

## 2. Intended Use

> **CardioCore V1 is a RESEARCH, PROTOTYPING, and EDUCATION platform ONLY.**

- It is **NOT a certified medical device** and has **no regulatory clearance or conformance**
  (no FDA, CE, ISO, or equivalent status is claimed or implied).
- It **must not** be used for diagnosis, treatment, patient monitoring, emergency use, or any
  clinical decision-making.
- It performs **raw signal acquisition only**. It does **not** interpret, classify, or clinically
  read ECG signals, and contains **no diagnostic algorithms**.
- Any human-subject experimentation must follow applicable ethics, safety, and electrical-isolation
  requirements and appropriate institutional review. Electrical isolation of any subject from
  mains-powered equipment is the responsibility of the experimenter; battery-only operation is
  strongly preferred during any signal capture from a person.

This document describes engineering architecture for that research context exclusively.

---

## 3. System Architecture Diagram (Text)

```
   ELECTRODES (10)                     ANALOG FRONT-END                 CONTROLLER                 INTERFACES
 ┌───────────────────┐         ┌──────────────────────────────┐   ┌─────────────────────┐
 │ RA  LA  LL         │         │            ADS1298           │    │                     │
 │ V1 V2 V3 V4 V5 V6  │ ───────▶│  8-ch, 24-bit, simultaneous  │    │                     │      ┌─────────────┐
 │  (signal inputs)   │  diff   │  - per-channel PGA           │    │                     │ ───▶│   BLE GATT  │
 └───────────────────┘  pairs  │  - lead-off detection        │    │                     │      │ (streaming) │
                                 │  - internal test signal      │    │                     │      └─────────────┘
 ┌───────────────────┐         │  - RLD amplifier ───────────┐│    │   ESP32-S3-WROOM-1   │
 │ RL / RLD electrode │◀────────┤  (right-leg drive output)   ││    │     -N16R8           │      ┌─────────────┐
 │ (driven reference) │  drive  │                              │    │  (SPI master,       │  ───▶│   microSD   │
 └───────────────────┘         │                              │    │   buffering, BLE,   │      │  (logging)  │
                                 │   SPI (SCLK/DIN/DOUT/CS)     │    │   SD, power mgmt)   │      └─────────────┘
                                 │   + DRDY, START, RESET/PWDN  │◀──▶│                     │
                                 └───────────────┬──────────────┘   │                     │      ┌─────────────┐
                                                 │ VREFP (2.5 V)     │                     │ ◀──▶│    USB-C    │
                                          ┌──────┴───────┐          │                     │      │ charge+prog │
                                          │   REF5025    │           └──────────┬──────────┘      └──────┬──────┘
                                          │  2.5 V ref   │                       │                        │
                                          └──────────────┘                      │                        │
                                                                                  │                        │
 POWER CHAIN                                                                      │                        │
 ┌───────────┐    ┌──────────────────┐    ┌───────────────────┐    ┌─────────────┴────────┐              │
 │  1S LiPo  │───▶│ Charger + Power-  │───▶│  3.3 V Regulator  │───▶│  3V3 digital rail    │              │
 │  (3.7 V)  │◀───│ Path (USB-C in)  │ ◀───┼───────────────────┤    │  + analog supply     │              │
 └───────────┘    └────────▲─────────┘    │  (AVDD / DVDD      │    │  (AVDD/AVSS to AFE)  │              │
                            │              │   separation)     │    └──────────────────────┘              │
                            └──────────────┴─────────── USB-C 5 V ◀──────────────────────────────────────┘
```

Signal flow summary:
`Electrodes → ADS1298 (digitize) → SPI → ESP32-S3 → {BLE, microSD, USB-C}`, with **REF5025**
supplying the ADS1298 voltage reference and the **1S LiPo → charger/power-path → 3.3 V regulator**
chain supplying power. The **RL/RLD** node is an *output* (driven reference), not a sense input.

---

## 4. Hardware Blocks

1. **Electrode interface / input protection** — connector(s) and per-input protection
   (series resistors, ESD/clamp, optional RF/DC-bias filtering) for the 10 electrode lines.
2. **ADS1298 analog front-end (AFE)** — 8-channel, 24-bit simultaneous-sampling ADC + PGA + RLD +
   lead-off detection + internal test signal.
3. **REF5025 precision reference** — external 2.5 V reference with decoupling/buffering into the
   ADS1298 VREFP/VREFN.
4. **ESP32-S3 controller** — MCU module (Wi-Fi/BLE capable; BLE used here), SPI master, buffering,
   logging, supervision.
5. **microSD interface** — card slot on a dedicated SPI bus (or SDIO — **TBD**) for local logging.
6. **USB-C subsystem** — connector, CC handling, USB data (programming / native USB-Serial-JTAG of
   the ESP32-S3), and 5 V input to the charger.
7. **Battery charger + power-path** — 1S LiPo charge management with USB present/absent power-path
   to the system load (part **TBD**).
8. **3.3 V regulation** — main 3.3 V rail with attention to analog vs digital supply quality.
9. **Clocking** — ADS1298 clock source (internal oscillator vs externally supplied clock — **TBD**,
   relevant for multi-device synchronization).
10. **Status / indication** — charge/status LEDs, optional user button(s).
11. **Expansion connector** — header(s) exposing shared SPI/clock/control for daisy-chaining
    additional ADS1298 devices (see Section 12).

---

## 5. ESP32-S3 Responsibilities

- **SPI master** — drives the ADS1298 SPI bus (SCLK, DIN/MOSI, DOUT/MISO, CS); manages control lines
  **DRDY** (data-ready interrupt), **START**, **RESET**, **PWDN**, and reads streamed conversion
  frames. DRDY is handled via a GPIO interrupt to avoid polling jitter.
- **Data buffering** — assembles 24-bit-per-channel frames into ring buffers in internal SRAM and/or
  **8 MB PSRAM**, decoupling acquisition timing from transport (BLE/SD) timing.
- **BLE GATT streaming** — exposes a custom GATT service to stream raw samples and report device
  state. Throughput and packetization are constrained by BLE MTU/connection interval; the design
  assumes batched/notification-based transfer (exact GATT layout **TBD**, see open questions).
- **microSD logging** — writes raw frames (with timestamps/sequence counters) to the card for
  offline analysis; manages filesystem, file rotation, and overflow behavior.
- **Power / charge status** — reads charger status and battery voltage (via ADC or fuel-gauge — part
  **TBD**); reports state of charge and charging status; manages low-battery behavior.
- **Configuration** — sets ADS1298 registers (sample rate, per-channel PGA gain, channel
  mux/test-signal selection, RLD configuration, lead-off detection settings) and persists user
  configuration.

The ESP32-S3 performs **acquisition, transport, logging, and control only** — it does **not**
interpret signals.

---

## 6. ADS1298 Responsibilities

- **8-channel simultaneous 24-bit acquisition** — each channel has its own delta-sigma ADC, so all
  channels sample at the same instant (essential for meaningful multi-lead ECG).
- **Internal PGA** — per-channel programmable gain (e.g., 1–12×, exact gains per datasheet) ahead of
  each ADC, configured by firmware to match electrode signal levels.
- **Internal test signal** — built-in square-wave/test signal source for bring-up, self-test, and
  end-to-end signal-path verification without a live subject.
- **Right-leg drive (RLD)** — integrated amplifier that derives a driven common-mode reference and
  feeds it back through the **RL/RLD** electrode to suppress common-mode interference (notably mains
  hum). RLD source/configuration is firmware-selectable.
- **Lead-off detection capability** — current-source/comparator-based detection of disconnected or
  high-impedance electrodes, reported via status bits in each data frame.
- **Data interface** — streams conversions over SPI with a DRDY signal to the ESP32-S3; supports
  daisy-chain modes relevant to expansion (Section 12).

The ADS1298 provides raw digitized leads only; no interpretation occurs in the AFE.

---

## 7. REF5025 Role

The **REF5025** provides a **precision external 2.5 V reference** to the ADS1298 reference inputs.

**Why external rather than the ADS1298's internal reference:**

- **Lower noise / better stability** — a dedicated precision reference typically offers tighter
  initial accuracy, lower temperature drift, and lower noise than the on-chip reference, directly
  improving the effective number of bits and baseline stability of a 24-bit acquisition.
- **Decoupling design freedom** — an external reference can be cleanly filtered/buffered and placed
  for quiet layout, isolating it from digital activity.
- **Determinism across devices** — when scaling to multiple ADS1298 devices (Section 12), a shared,
  well-controlled reference helps keep inter-device gain matching consistent.

REF5025 output is generously decoupled and routed as a quiet analog net into VREFP (with VREFN
referenced to analog ground per the ADS1298 reference scheme). The 2.5 V choice matches a common
ADS1298 reference operating point; final bypass/buffer network values are **TBD** pending bring-up.

---

## 8. Power System

- **Source:** single-cell **1S LiPo** (~3.0–4.2 V). Pack capacity **TBD** based on target runtime.
- **USB-C charging:** 5 V from USB-C feeds a **dedicated LiPo charger** (charge current and part
  **TBD**). Charging and acquisition coexistence (charge-while-running) policy is **TBD**.
- **Power-path:** a power-path front end selects USB vs battery so the system can run from USB while
  charging and switch seamlessly to battery; specific charger/power-path IC is **TBD**.
- **3.3 V rail:** main system regulator producing the 3V3 digital rail for the ESP32-S3, microSD,
  and digital side of the ADS1298 (DVDD). Regulator type (buck vs LDO) is **TBD**; an LDO or a
  low-noise post-regulator is favored for the analog domain to minimize switching noise.
- **Analog/digital supply separation:** the ADS1298 distinguishes AVDD (analog) and DVDD (digital).
  The design intent is to derive a **clean analog supply** (AVDD/AVSS) for the AFE and reference —
  ideally from an LDO and ferrite-isolated net — kept separate from noisy digital switching rails.
  Whether analog runs single-supply or bipolar (relative to mid-supply) is **TBD** and depends on
  the chosen ADS1298 supply configuration.
- **Battery sensing:** battery voltage / state-of-charge sensing via ESP32-S3 ADC divider or a
  dedicated fuel gauge — part and method **TBD**.

All exact part numbers above are **TBD** and must be confirmed against availability, noise, and
thermal budgets before layout.

---

## 9. Electrode Interface

Ten electrode inputs are supported, mapped to a standard limb + precordial set. The ADS1298 has 8
differential input channels (IN1P/IN1N … IN8P/IN8N). One common reference strategy uses **Wilson
central terminal (WCT)** / a chosen reference node so each precordial lead is measured against a
defined reference; the exact wiring (WCT vs single-ended-to-reference) is **TBD** and configured in
firmware via the ADS1298 input mux.

| Electrode | Role                       | Notes                                                  |
|-----------|----------------------------|--------------------------------------------------------|
| RA        | Right Arm (limb)           | Used to derive limb leads / WCT                        |
| LA        | Left Arm (limb)            | Used to derive limb leads / WCT                        |
| LL        | Left Leg (limb)            | Used to derive limb leads / WCT                        |
| RL / RLD  | **Driven reference (RLD)** | **Output**, not a sense input; driven by ADS1298 RLD   |
| V1        | Precordial                 | → ADS1298 channel input (mapping **TBD**)              |
| V2        | Precordial                 | → ADS1298 channel input (mapping **TBD**)              |
| V3        | Precordial                 | → ADS1298 channel input (mapping **TBD**)              |
| V4        | Precordial                 | → ADS1298 channel input (mapping **TBD**)              |
| V5        | Precordial                 | → ADS1298 channel input (mapping **TBD**)              |
| V6        | Precordial                 | → ADS1298 channel input (mapping **TBD**)              |

Notes:
- **RL/RLD is the driven right-leg reference**, sourced from the ADS1298 RLD amplifier — it actively
  drives common-mode rejection and is **not** sampled as a signal.
- The 8 simultaneous channels are allocated across the precordial and/or limb-derived leads; the
  precise channel-to-electrode mapping (and how 6 precordial + limb leads map onto 8 channels) is a
  firmware/configuration decision marked **TBD**.
- Each input includes series protection and high-impedance-friendly filtering; component values are
  **TBD** pending input-network design.

---

## 10. Layout Requirements (High Level)

- **Analog/digital separation:** physically partition the AFE/reference (analog) region from the
  ESP32-S3, microSD, and switching/digital region. Keep digital return currents out of the analog
  area.
- **Star / single-point grounding:** join analog ground and digital ground at a single, controlled
  point (or via a deliberate stitching strategy), avoiding ground loops that inject noise into the
  AFE.
- **Keep the reference and AFE quiet:** place REF5025 and its decoupling close to the ADS1298
  reference pins; route VREF as a short, guarded, low-noise net away from clocks, SPI, BLE RF, and
  the switching regulator.
- **Short high-impedance traces:** keep electrode/input traces (high source impedance) short,
  symmetric (matched lengths for differential pairs), and shielded; minimize exposure to noisy nets.
- **Guard rings:** use guard rings/traces around high-impedance input nodes and the reference to
  reduce leakage and coupling.
- **RF keep-out:** respect the ESP32-S3 module antenna keep-out; keep the antenna away from the
  analog section and ground pour as the module datasheet requires.
- **Decoupling discipline:** local bypassing on every supply pin (AVDD, DVDD, VREF) with short loops.

These are intent-level guidelines; detailed stackup, impedance, and placement are **TBD** at layout.

---

## 11. Expansion Strategy (Conceptual)

CardioCore V1 fixes the 8-channel base; CardioVest targets higher channel counts (16 / 32 / 64).
The conceptual scaling path:

- **Daisy-chained ADS1298 devices:** the ADS1298 supports daisy-chain operation where multiple
  devices share the **SPI clock and control lines** and cascade their DOUT, so the ESP32-S3 reads N
  devices as one extended frame.
  - 16 channels = 2× ADS1298
  - 32 channels = 4× ADS1298
  - 64 channels = 8× ADS1298
- **Shared clock for synchronization:** to keep *all* channels truly simultaneous across devices, the
  ADS1298s should share a **common clock** (externally supplied) and synchronized **START/RESET**.
  This is the main reason the V1 clock source is flagged **TBD** — a future common-clock distribution
  scheme must be feasible.
- **Shared reference:** a single REF5025 (suitably buffered) or matched references across devices to
  keep inter-device gain consistent.
- **Connector / stacking strategy:** an **expansion connector** carries shared SPI/clock/control/
  power so additional AFE boards can be **stacked or ribbon-connected**. Mechanical/connector choice
  and how many devices one ESP32-S3 SPI bus can sustain at the target sample rate are **TBD**.
- **Data-rate scaling:** higher channel counts multiply SPI throughput and BLE/SD bandwidth demands;
  PSRAM buffering and possibly bulk transport (USB/SD-first at high channel counts) become more
  important — quantification is **TBD**.

This section is conceptual; no expansion hardware is committed in V1 beyond exposing the bus.

---

## 12. Open Engineering Questions

1. **Clock source:** ADS1298 internal oscillator (simplest for V1) vs external clock (required for
   clean multi-device synchronization later). Which to commit to for V1?
2. **Channel mapping:** exact assignment of 6 precordial + limb-derived leads onto the 8 ADS1298
   channels, and WCT vs direct-reference wiring.
3. **microSD bus:** dedicated SPI vs SDIO, and whether it shares or contends with the AFE SPI bus.
4. **BLE throughput:** achievable sustained sample rate × channels over BLE GATT; required MTU,
   connection interval, and packetization; fallback to SD/USB at high rates.
5. **Power-path / charger / regulator parts:** specific ICs, charge current, charge-while-acquire
   policy, and analog-supply generation (LDO vs post-regulated buck).
6. **Analog supply topology:** single-supply vs bipolar operation of the ADS1298 and resulting input
   common-mode range.
7. **Battery sensing:** ADC divider vs dedicated fuel gauge.
8. **Input protection network:** series R, clamp, and filter values balancing safety, bandwidth, and
   input impedance.
9. **Timestamping:** sample-accurate timebase shared across BLE and SD records.
10. **Mechanical/expansion connector:** pinout, stacking vs ribbon, max devices per SPI bus.

---

## 13. Next Steps

1. Lock the ADS1298 **clock decision** (internal vs external) since it gates the expansion strategy.
2. Define the **channel-to-electrode mapping** and reference (WCT) scheme in firmware terms.
3. Select **power-path / charger / 3.3 V regulator** candidate parts and draft the power budget
   (resolve relevant **TBD**s).
4. Draft the **ADS1298 register configuration** (sample rate, PGA, RLD, lead-off) and a bring-up
   plan using the internal test signal.
5. Define the **BLE GATT service** layout and the **SD log format** (frame structure, timestamps,
   sequence counters, status/lead-off bits).
6. Produce a preliminary **schematic** for the AFE + reference + power chain for review.
7. Plan the **layout partition** (analog/digital regions, star ground, REF/AFE placement) ahead of
   board layout.
8. Define a **bench validation plan** (test-signal verification, noise floor, lead-off behavior)
   using signal generators / phantoms — **no human subjects** until safety/isolation is reviewed.

---

*This is a research/education engineering document for CardioCore V1. It describes a non-medical,
uncertified prototype and makes no diagnostic, clinical, or regulatory claims.*
