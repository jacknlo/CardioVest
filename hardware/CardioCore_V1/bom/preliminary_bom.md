# CardioCore V1 — Preliminary Bill of Materials (BOM)

> **Status: PRELIMINARY.** This BOM is an early-stage planning document. Part numbers,
> quantities, footprints, and supplier references are **subject to change** as the
> schematic, layout, and component selection mature. Items marked **TBD** ("to be
> determined") have not yet been finalized and must not be treated as committed values.

> **Research / education use only.** CardioCore V1 is a research, prototyping, and
> educational hardware platform. It is **not** a certified medical device and is not
> intended for diagnosis, treatment, patient monitoring, emergency use, or any clinical
> decision-making. No regulatory conformance (e.g., FDA / CE / ISO) is claimed.

---

## Scope

This BOM covers the **CardioCore V1** main acquisition board: an 8-channel, 24-bit,
battery-powered ECG acquisition board. It includes the core MCU, analog front-end (AFE),
precision voltage reference, power/charging subsystem, storage, connectivity, and the
electrode-input front-end protection. Mechanical hardware (enclosure, fasteners),
electrodes/lead-sets, and assembly consumables are **out of scope** for this revision.

---

## BOM Table

| Ref / Category | Part / Description | Example MPN | Qty | Notes |
|---|---|---|---|---|
| U1 — MCU module | ESP32-S3-WROOM-1, 16 MB flash / 8 MB PSRAM; BLE + Wi-Fi, USB-OTG, SPI/I2C | ESP32-S3-WROOM-1-N16R8 | 1 | Hosts firmware, BLE streaming, SD logging. Reserve dedicated SPI bus for ADS1298; keep RF keep-out per module datasheet. |
| U2 — ECG AFE | 8-channel, 24-bit simultaneous-sampling analog front-end (ECG), TQFP-64 | ADS1298IPAG | 1 | Core acquisition device. Integrated PGA, right-leg drive (RLD), lead-off detection. Needs clean analog supply + low-noise reference. |
| U3 — Voltage reference | 2.5 V precision, low-noise series voltage reference | REF5025 (REF5025AIDR / REF5025IDR) | 1 | External reference for ADS1298 to improve noise/stability vs. internal ref. Confirm package (SOIC-8) and grade. TBD: A vs. non-A grade. |
| U4 — Charger / power-path | 1S LiPo linear charger with USB-C input and power-path (system load sharing) | MPN TBD | 1 | Single-cell (1S) Li-ion/LiPo charge management. Power-path preferred so board runs while charging. Charge current setpoint TBD. |
| U5 — 3.3 V regulator | Low-noise / low-dropout LDO, 3.3 V output (analog-friendly) | MPN TBD | 1 | Low-noise LDO preferred to feed analog/AFE rail. Consider separate analog vs. digital 3.3 V rails. Output current budget TBD. |
| J1 — microSD socket | microSD card socket, push-push, hinged/SMD | MPN TBD | 1 | For local data logging. Push-push mechanism. SPI or SDMMC interface — interface choice TBD. |
| J2 — USB-C connector | USB Type-C receptacle (charging + programming/data) | MPN TBD | 1 | USB-C for charging and programming/native-USB. Include CC pull-downs (Rd) for sink role. ESD on data lines (see D-array). |
| J3 — Electrode connector | ECG electrode input connector, 10 inputs: RA, LA, LL, RL/RLD, V1–V6 | MPN TBD | 1 | 10-electrode 12-lead-style input set. Connector style (e.g., keyed header vs. medical-style) TBD. RL = right-leg drive return. |
| D-array — ESD protection | Low-leakage, low-capacitance ESD protection array for electrode inputs | MPN TBD | 1–2 | **Low input leakage current is critical** to avoid corrupting high-impedance ECG signals. Quantity depends on channels per array. |
| Passives | Resistors, capacitors (incl. AFE input RC/filters, decoupling), ferrites/beads | Various | TBD | Grouped placeholder line. Values, tolerances, and counts **TBD** pending schematic + layout. Includes RLD network, input filters, supply decoupling. |
| J4 — Battery connector | 1S LiPo battery connector (2-pin) | JST PH (2-pin) — exact MPN TBD | 1 | Mates to 1S LiPo pack. Confirm series/pitch (e.g., PH 2.0 mm) and polarity/keying. Add reverse-polarity protection — TBD. |

---

## Open BOM Questions

- **Charger IC (U4):** Select specific 1S charger with power-path; finalize charge
  current, thermal regulation, and whether fuel-gauge / battery monitoring is needed.
- **LDO topology (U5):** One shared 3.3 V rail vs. separate low-noise analog rail +
  digital rail? Decide regulator part(s) and decoupling strategy for AFE noise targets.
- **Voltage reference grade (U3):** REF5025 grade/initial-accuracy and whether a
  reference buffer / extra filtering is required at the ADS1298 reference pins.
- **microSD interface (J1):** SPI vs. SDMMC — affects pin allocation on the ESP32-S3
  and achievable logging throughput.
- **Electrode connector (J3):** Final connector type and keying for the 10 inputs
  (RA, LA, LL, RL/RLD, V1–V6); strain relief and shielding approach.
- **Input protection (D-array):** Confirm low-leakage / low-capacitance ESD parts and
  whether series current-limiting + defibrillation-style protection is in scope (note:
  this is a research board, not a clinical device).
- **Passives (grouped line):** Expand into a fully itemized sub-BOM once the schematic
  is frozen (RLD network, anti-alias/input filters, decoupling, ferrites).
- **Battery connector (J4):** Confirm JST series/pitch and add reverse-polarity and
  over-current protection.
- **Sourcing / second sources:** Identify alternate MPNs and stock/lead-time risk for
  each long-lead component (especially U2 AFE and U1 module).
