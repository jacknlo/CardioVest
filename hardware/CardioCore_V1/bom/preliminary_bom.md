# CardioCore V1 — Bill of Materials (BOM)

> **Status: WORKING DRAFT — derived from the flux.ai design.** This BOM is
> generated from the CardioCore V1 flux.ai project export (board:
> *"wearable 8-channel ECG acquisition board"*, BOM export dated **2026-06-30**).
> It reflects the parts currently chosen in the schematic, but values and part
> numbers may still change before PCB layout. Items marked **TBD** are not yet
> finalized and must not be treated as committed values.

> **Research / education use only.** CardioCore V1 is a research, prototyping, and
> educational hardware platform. It is **not** a certified medical device and is not
> intended for diagnosis, treatment, patient monitoring, emergency use, or any clinical
> decision-making. No regulatory conformance (e.g., FDA / CE / ISO) is claimed.

---

## Source of truth

The authoritative, machine-readable BOM lives in [`flux_export/`](./flux_export/),
exported directly from flux.ai in multiple vendor formats:

- [`BOM_Flux.csv`](./flux_export/BOM_Flux.csv) — full native export (all attributes).
- `BOM_JLCPCB.csv`, `BOM_PCBWay.csv`, `BOM_Elecrow.csv`, `BOM_Seeed.csv`,
  `BOM_AllPCB.csv`, `BOM_AdvancedCircuits.csv`, `BOM_Eurocircuits.csv` — per–fab-house
  assembly formats.
- [`CardioCore_V1_BOM_flux_export.zip`](./flux_export/CardioCore_V1_BOM_flux_export.zip) — the original export archive.

The editable design source is [`../flux_project/CardioCore_V1.flx`](../flux_project/CardioCore_V1.flx)
and the schematic netlist is [`../schematics/CardioCore_V1.edif`](../schematics/CardioCore_V1.edif).
This Markdown file is a human-readable summary; if it ever disagrees with the flux.ai
export, the export wins.

---

## Scope

The **CardioCore V1** main acquisition board: an 8-channel, 24-bit, battery-powered ECG
acquisition board. Covers the MCU, analog front-end (AFE), precision voltage reference,
power/charging subsystem, storage, connectivity, electrode-input protection, and an
expansion header. Mechanical hardware (enclosure, vest integration), electrodes/lead-sets,
and assembly consumables are **out of scope** for this revision.

**Total: 64 placed components across 19 line items.**

---

## BOM Table

| Designator(s) | Qty | Part / Description | MPN | LCSC | Package | Role |
|---|---|---|---|---|---|---|
| **U1** | 1 | ESP32-S3 module — Wi-Fi/BLE, 16 MB flash / 8 MB PSRAM, native USB (Espressif) | `ESP32-S3-WROOM-1-N16R8` | — | Module | Wireless MCU: BLE streaming, ADS1298 + microSD SPI control, USB debug/program |
| **U2** | 1 | 8-channel, 24-bit simultaneous-sampling ECG AFE (TI) | `ADS1298IPAGR` | C527440 | TQFP-64 | ECG AFE: 8 differential channels, SPI, RLD drive, lead-off, internal test signal |
| **U3** | 1 | 1S LiPo charger / power-path IC (Monolithic Power Systems) | `MP2662` | — | WLCSP-9 | Battery charger: USB-C 5 V LiPo charging + power-path (charge current set conservatively) |
| **U4** | 1 | 3.3 V LDO, 600 mA, low-noise (Diodes Inc.) | `AP2112K-3.3TRG1` | — | SOT-23-5 | 3.3 V rail (~340 mA active + ESP32 transient margin) |
| **U5** | 1 | 2.5 V precision voltage reference (TI) | `REF5025IDGK` | — | VSSOP-8 (DGK) | Low-noise external reference for ADS1298; place close to AFE, decouple per datasheet |
| **D1, D2** | 2 | 5-channel low-capacitance ESD protection array | `PESD3V3L5UY` | — | SOT-363 (SC-70-6) | Electrode ESD protection at connector, ahead of AFE filters *(PESD3V3L5UY is a **Nexperia/NXP** part — the flux export mislabels the mfr as TI; verify part/availability)* |
| **J1** | 1 | USB-C receptacle, 16-pin (CIKI) | `TYPE-C-2.0-16PIN-SMT-3-OR` | C2987387 | SMD | Sink-only USB-C: charging + USB2 D+/D− to ESP32-S3 native USB |
| **J2** | 1 | microSD card socket (CONNFLY) | `DS1139-06-08SS4BSR` | C86574 | SMD | SPI-mode microSD logging (local decoupling + pull-ups) |
| **J3** | 1 | ECG electrode connector (Molex) | `5034801000` | — | — | 10 electrodes: RA, LA, LL, RL/RLD, V1–V6 → ESD/protection/filter → ADS1298 |
| **J4** | 1 | Battery connector, JST-PH (JST) | `B2B-PH-K-S` | — | THT | 1S LiPo input — **battery-powered only** |
| **J5** | 1 | Expansion header, 2×20, 2.54 mm female (ZHOURI) | `2.54-2*20` | C2977589 | THT | Expansion: 3V3, GND, SPI, I2C, sync/control GPIO + spare GPIO for additional ECG-channel modules |
| **SW1, SW2** | 2 | Tactile switch (XUNPU) | `TS-1088-AR02016` | C720477 | SMD | SW1 = EN/reset, SW2 = BOOT (GPIO0) — per netlist |
| **C1–C14** | 14 | Non-polarized capacitor, 100 nF, 16 V | — | — | 0402 | Supply decoupling |
| **C15–C30** | 16 | Non-polarized capacitor, **value TBD**, 16 V | — | — | 0402 | **TBD** — input CM caps (C15–C24) + differential caps (C25–C29) + RLD cap (C30); prefer low-leakage C0G/NP0 |
| **R1, R2** | 2 | Resistor, 5.1 kΩ | — | — | 0402 | USB-C CC1/CC2 sink pull-downs to GND |
| **R3, R4, R5** | 3 | Resistor, 10 kΩ | — | — | 0402 | Pull-ups: GPIO0 boot, ADS1298 CS (default inactive), ESP32 EN |
| **R6, R7** | 2 | Resistor, 4.7 kΩ | — | — | 0402 | I2C SDA/SCL pull-ups (charger / expansion) |
| **R8** | 1 | Resistor, 100 kΩ | — | — | 0402 | MP2662 NTC bias placeholder *(verify charger datasheet)* |
| **R9–R20** | 12 | Resistor, **value TBD** | — | — | 0402 | **TBD** — ECG input series resistors (R9–R18); R19/R20 are the RLD network |

---

## ⚠ Required additions before layout — bulk capacitors MISSING

The current export has **no bulk capacitance**: C1–C14 are all 100 nF and C15–C30 are
the input/RLD network. This is a **blocking gap** — the AP2112K LDO requires Cin/Cout ≥ 1 µF
ceramic to be stable, and there is no bulk on the charger, VSYS, USB 5 V, VREFP, or VCAP1.
Add the following to the flux.ai schematic (values candidate — confirm against each
datasheet) and re-export before PCB layout:

| Node | Proposed | Notes |
|---|---|---|
| AP2112K (U4) Vin | 1 µF X7R (+ optional 10 µF bulk) | LDO input stability |
| AP2112K (U4) Vout / 3V3 rail | 1 µF X7R min (2.2–10 µF bulk recommended) | LDO output stability + ESP32 BLE-TX transient (350–500 mA) |
| MP2662 (U3) VIN / SYS | 1–10 µF per datasheet | charger input/system bulk |
| USB-C 5 V (J1) | 1–10 µF | input bulk / hot-plug |
| ADS1298 VREFP (from REF5025) | ~1 µF + 100 nF | reference decoupling (currently only 100 nF) |
| ADS1298 VCAP1 | ~1 µF per SBAS459 | internal reference cap |

See finding **4.1** in the software/hardware review and **§7** of the AFE Verification
Report. Until these exist in the export, the component count and power tree are incomplete.

---

## Power chain summary

```text
USB-C (J1, 5 V) ──▶ MP2662 charger/power-path (U3) ◀──▶ 1S LiPo (J4)
                          │
                          ▼
                 AP2112K-3.3 LDO (U4) ──▶ 3.3 V rail ──▶ ESP32-S3 (U1), ADS1298 (U2, digital)

REF5025 (U5) ──▶ 2.5 V reference ──▶ ADS1298 (U2) VREFP
```

---

## Open BOM questions

- **ECG input network (series R9–R18, CM C15–C24, diff C25–C29; R19/R20/C30 = RLD):** finalize the per-channel series-resistor and
  filter-capacitor values from ADS1298 application guidance, input leakage/noise budget,
  and target ECG bandwidth **before** PCB layout. These are the last electrically critical
  TBDs (see [`../../../docs/ADS1298_Analog_Frontend_Notes.md`](../../../docs/ADS1298_Analog_Frontend_Notes.md)).
- **ESD array (D1, D2):** confirm the manufacturer and exact part for `PESD3V3L5UY`
  (the flux export attributes it inconsistently); verify it is low-leakage and
  low-capacitance enough for high-impedance ECG inputs.
- **Charger setpoints (U3 / R8):** confirm the MP2662 charge current and NTC/thermistor
  network against the chosen LiPo cell datasheet.
- **Reference filtering (U5):** confirm whether the REF5025IDGK needs an additional
  buffer / extra filtering at the ADS1298 reference pins for the target noise.
- **Electrode connector (J3):** the Molex `5034801000` is a **0.50 mm FFC/FPC flat-flex**
  connector — the electrode harness must terminate in a matching flat-flex cable/breakout.
  Confirm the mating, strain relief, and
  shielding approach for the 10-electrode harness.
- **Sourcing / second sources:** identify alternates and stock/lead-time risk for the
  long-lead parts (especially U2 AFE and U1 module).
- **Mechanical / vest integration:** out of scope here; track separately once the board
  revision is frozen.
