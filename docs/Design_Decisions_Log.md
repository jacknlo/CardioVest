# Design Decisions Log — CardioCore V1

## Purpose

This log records the significant architecture and design decisions made for the
**CardioCore V1** board (part of the **CardioVest** research platform). It is a
lightweight, ADR-style record: each row captures *what* was decided, *why*, its
current *status*, and any caveats or links. The goal is a durable, append-only
history so that anyone (including future-us) can understand the reasoning behind
the current design without reconstructing it from scratch.

**How to add an entry:** append a new row to the table below. Give it a date,
state the decision in one line, give a concise real reason, set a status, and add
notes (links to the relevant blocker in
[`Blockers_Before_PCB_Layout.md`](./Blockers_Before_PCB_Layout.md), GitHub issues,
or datasheet references). Never edit or delete past rows to "fix" history — if a
decision changes, add a new row that supersedes the old one and note the
supersession in both rows.

## Decisions

| Date | Decision | Reason | Status | Notes |
|------|----------|--------|--------|-------|
| 2026-06-29 | **D1** — Repository / platform name: **CardioVest** | Single umbrella name for the wearable multi-channel ECG research platform; boards are modules under it. | Accepted | Public repo: github.com/jacknlo/CardioVest |
| 2026-06-29 | **D2** — First board name: **CardioCore V1** | Names the core acquisition board distinctly from the platform so future boards/revisions version cleanly. | Accepted | Spec: `hardware/CardioCore_V1/Project_Specification.md` |
| 2026-06-29 | **D3** — MCU: **ESP32-S3-WROOM-1-N16R8** | Certified module with integrated antenna and shielding avoids discrete RF design risk; 16 MB flash + 8 MB PSRAM and native USB suit multi-channel buffering and on-board logging. | Accepted | U1. Antenna keepout still applies at layout (B14). |
| 2026-06-29 | **D4** — ECG AFE: **ADS1298 (ADS1298IPAGR, TQFP-64)** | Purpose-built 8-channel, 24-bit simultaneous-sampling ECG front end with integrated RLD, lead-off detection and SPI; removes need for discrete instrumentation amplifier chains. | Accepted | U2. Input network, decoupling and RLD still TBD (B1, B3, B5). |
| 2026-06-29 | **D5** — External reference: **REF5025 (REF5025IDGK, 2.5 V)** | Low-noise, low-drift external reference gives a cleaner, more stable ADC reference than the AFE internal reference, improving channel noise floor for research-grade acquisition. | Accepted | U5. Standalone vs buffered topology + decoupling TBD (B4). |
| 2026-06-29 | **D6** — Initial channel count: **8 ECG channels** | Matches the ADS1298 native channel count and supports a standard limb + V1–V6 electrode set without cascading multiple AFEs in V1. | Accepted | Electrode-to-channel mapping + WCT handling TBD (B8). |
| 2026-06-29 | **D7** — Charger / power-path: **MP2662**; 3.3 V LDO: **AP2112K-3.3 (600 mA)** | Integrated single-cell Li-ion charger with power-path enables charge-while-program off USB; low-noise LDO provides a clean 3.3 V rail with adequate headroom for MCU + AFE. | Accepted | U3 / U4. Battery capacity, charge current and NTC values TBD (B11). |
| 2026-06-29 | **D8** — Electrode ESD: **PESD3V3L5UY** low-capacitance arrays | Low-capacitance, multi-channel ESD protection on electrode inputs guards the AFE without loading the signal path; 5-channel arrays match the connector channel grouping. | Accepted | D1, D2. Reverse-leakage / capacitance to be VERIFIED against datasheet (B6). |
| 2026-06-29 | **D9** — Power workflow: **battery-only during body-connected measurement** (USB for charge/program only) | Removing non-isolated mains-derived USB during any body-connected test is the core electrical-safety research practice for this prototype; enforced by a USB-present / AFE-enable interlock concept. | Accepted | Interlock realization is TBD (B9, B10). |
| 2026-06-29 | **D10** — Board size target **≤ 60 × 60 mm**; **4-layer** stackup (preferred) | Compact size suits a wearable form factor; 4-layer (top signal / GND / power+quiet-analog / bottom signal) gives a solid reference plane and analog/digital separation for low-noise acquisition. | Accepted (target) | Exact stackup / impedance to be FINALIZED at layout (B13); routing constraints B15. |
| 2026-06-29 | **D11** — EDA tool: **flux.ai** | Chosen design environment for this project; source and netlist are maintained in flux format. | Accepted | Source: `hardware/CardioCore_V1/flux_project/CardioCore_V1.flx`; netlist: `schematics/CardioCore_V1.edif`. Not KiCad. |
| 2026-06-29 | **D12** — Device status: **research / prototyping / education only** | Scopes the project away from medical-device claims and regulatory obligations; all design choices are made for a research instrument, not a clinical product. | Accepted | Not a medical device. See research-use note below. |

## How to record future decisions

- **Append, don't rewrite.** Add new rows at the bottom of the table. Do not
  alter the text of an existing row to change its meaning; history is immutable.
- **Supersede explicitly.** When a new decision replaces an older one, add a new
  row, set the old row's Status to `Superseded by Dn`, and reference the old `Dn`
  in the new row's Notes.
- **Keep reasons real and concise.** One or two sentences on the actual
  engineering trade-off, not a restatement of the decision.
- **Use clear statuses.** e.g. `Accepted`, `Accepted (target)`, `Provisional`,
  `Proposed`, `Superseded by Dn`, `Rejected`.
- **Link out.** Reference the corresponding blocker (B-number) in
  [`Blockers_Before_PCB_Layout.md`](./Blockers_Before_PCB_Layout.md), the GitHub
  issue, and any datasheet/section that justifies the choice.
- **Number sequentially.** Next decision is `D13`, then `D14`, and so on; never
  reuse a retired number.

## Research-use only

CardioVest / CardioCore V1 is a **research, prototyping, and education** project.
It is **not a medical device** and makes **no claim** of diagnosis, treatment,
patient monitoring, emergency use, or clinical decision-making, and **no**
regulatory status (FDA / CE / ISO) is claimed or implied. All decisions in this
log are recorded in that context.
