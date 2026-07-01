# Blockers Before PCB Layout — CardioCore V1

This document is the authoritative, single-source list of everything that **must** be resolved before PCB layout begins for **CardioCore V1**, the 8-channel battery-powered ECG acquisition board in the CardioVest platform. Items here are derived from the analog-front-end review, the project specification, and the preliminary BOM. Each item carries an ID, a short rationale (why it blocks layout / what must be decided), a status, and where relevant a pointer to the supporting document.

> **PCB layout is BLOCKED until the ADS1298 analog front-end is verified and the items below are resolved.**

Status legend:

- **TBD** — value or design decision not yet made; depends on datasheet verification and schematic review.
- **VERIFY** — a part is placed, but a datasheet-level parameter must be confirmed before it is locked.
- **FINALIZE** — direction is set; the exact figures (stackup, impedance, pinout) must be pinned down.
- **CONSTRAINT** — a routing/placement rule to apply during layout (not a value to compute now).
- **CONFIRM** — a part/choice is placed; confirm a detail (mating, manufacturer, cable) before locking.
- **DECIDE** — a design decision is pending (e.g., the channel montage); pick a direction.
- **FIX** — a known defect that must be corrected (treated as critical).

Related documents:

- `docs/AFE_Verification_Report.md` — **verified netlist connectivity, channel map, findings (F1–F11), and candidate values** (start here).
- `docs/Flux_Change_List.md` — punch list of schematic edits to apply when flux.ai access returns (~late July 2026).
- `hardware/CardioCore_V1/Project_Specification.md` — board targets, power workflow, safety interlock concept.
- `docs/ADS1298_Analog_Frontend_Notes.md` — AFE input network, decoupling, reference, RLD/DRL.
- `bom/preliminary_bom.md` — placed parts and reference designators.

---

## Group A — Analog front-end (ADS1298 input path)

| ID | Title | Why it blocks layout / what must be decided | Status | Reference |
|----|-------|---------------------------------------------|--------|-----------|
| B1 | ADS1298 input filter cap values (C15–C24 common-mode, C25–C29 differential) | Anti-alias / EMI filter caps set input bandwidth and CMRR balance at every channel; routing/footprints cannot be committed until values are chosen. Candidate values proposed in the AFE report. | TBD | `AFE_Verification_Report.md` |
| B2 | Series input resistor values (R9–R18) | Per-line series resistors form the input filter with B1 and limit fault current (note: **no** defibrillation protection is claimed/designed); value drives noise, footprint, and placement near the connector. | TBD | `AFE_Verification_Report.md` |
| B17 | ADS1298 clock strap (CLKSEL) | 🔴 **Critical:** CLKSEL is tied to **GND** (external-clock mode) but no clock source / CLK-pin connection exists — the device will not clock as wired. Re-strap CLKSEL **high** (internal oscillator) or add a clock source. | FIX 🔴 | `AFE_Verification_Report.md` (F1) |
| B8 | Lead / channel mapping & montage (electrode → INxP/INxN, WCT) | Mapping is now **extracted** from the netlist (5 differential pairs: RA-LA, LL-V1, V2-V3, V4-V5, V6-AUX; IN6–8 on J5). **Decide** whether this differential montage is intended, or whether precordials should be WCT-referenced. | DECIDE | `AFE_Verification_Report.md` (F2) |
| B22 | Input coupling (AC vs DC), target ECG bandwidth/sample rate, and analog supply scheme (unipolar vs bipolar) | 🟠 **Upstream of B1/B2/B3.** These decisions set every input R/C value, the common-mode window, and whether mid-supply bias resistors / AC-coupling caps are even needed. With DC coupling on a 3.3 V unipolar supply and no CM bias, the inputs are undefined at power-up/lead-off and a ±300 mV half-cell offset can saturate the PGA. Decide coupling + supply topology (and confirm the PGA common-mode range at the intended gain) **before** picking B1/B2 values. | DECIDE 🟠 | `ADS1298_Analog_Frontend_Notes.md` (§1, §9) |
| B18 | ADS1298 `RESV1` floating | 🔴 **Critical:** `RESV1` (U2) is in no net; TI SBAS459 requires `RESV1` → **DVDD**. Floating, the device may not configure/convert reliably. Tie `RESV1` → DVDD (3V3) before layout. | FIX 🔴 | `AFE_Verification_Report.md` (F9) |
| B19 | ADS1298 floating control pins (`DAISY_IN`, `GPIO1–4`, `WCT`, `TESTP/TESTN`) | These pins are in no net. Tie `DAISY_IN` → DGND (or per `CONFIG1.DAISY_EN`), define `GPIO1–4` (pull or firmware-driven); `WCT`/`TESTP`/`TESTN` may remain NC. Resolve before layout. | FIX | `AFE_Verification_Report.md` (F11) |

## Group B — Reference & decoupling

| ID | Title | Why it blocks layout / what must be decided | Status | Reference |
|----|-------|---------------------------------------------|--------|-----------|
| B4 | REF5025 reference implementation + decoupling | Decide standalone vs. buffered REF5025 (U5) and the associated capacitor values; this governs the reference node placement and the quiet-analog plane routing. | TBD | `ADS1298_Analog_Frontend_Notes.md` |
| B5 | ADS1298 decoupling (AVDD, AVSS, DVDD, VCAP1–4, VREFP, VREFN) | Decoupling values/placement around U2 are critical to ADC noise. **VREFP (C9) and VCAP1 (C10) need µF-range caps, not the 100 nF placeholder (F3)**. (VREFN → AVSS is already tied — F4 confirmed OK.) | TBD | `AFE_Verification_Report.md` (F3, F4) |
| B3 | RLD/DRL network values and loop stability | The Right-Leg-Drive feedback network (Rf, Cf, output series R) and its loop stability define the common-mode rejection path; component values and the driven-electrode return route must be settled first. | TBD | `ADS1298_Analog_Frontend_Notes.md` |
| B20 | Analog-supply isolation from the shared 3V3 rail | 🟠 AVDD/AVDD1 and REF5025 `Vin` share 3V3 with the ESP32-S3 and **no ferrite/filtered analog feed** exists; BLE-TX transients couple into the AFE. Add a ferrite-isolated analog branch (FB + local bulk) — or, for a 24-bit AFE, a dedicated low-noise analog LDO post-regulator — or explicitly accept/document the shared-rail risk. | FIX 🟠 | `AFE_Verification_Report.md` (F10) |
| B21 | Bulk capacitors missing from the export | 🔴 **Critical:** the BOM has **no bulk capacitance** (C1–C14 are all 100 nF). The AP2112K LDO needs Cin/Cout ≥ 1 µF ceramic to be stable, and there is no bulk on the charger, VSYS, USB 5 V, VREFP, or VCAP1. Add the bulk caps (see `bom/preliminary_bom.md` → "Required additions before layout") and re-export before layout. | FIX 🔴 | `bom/preliminary_bom.md` |
| B23 | 3V3 rail headroom over the 1S LiPo discharge curve | 🟠 AP2112K dropout (~250 mV @ 600 mA) needs Vbat ≳ 3.55 V; a 1S LiPo runs down to ~3.0 V, so AVDD and the REF5025 sag near end-of-charge, degrading the 24-bit AFE noise/gain. Define the battery cut-off voltage and confirm AVDD stability across discharge (consider a buck-boost VSYS or a lower analog rail with dropout margin). | DECIDE 🟠 | `AFE_Verification_Report.md` |

## Group C — Protection & connector

| ID | Title | Why it blocks layout / what must be decided | Status | Reference |
|----|-------|---------------------------------------------|--------|-----------|
| B6 | Low-leakage ESD device selection | PESD3V3L5UY arrays (D1, D2) are placed, but reverse-leakage and capacitance must be verified so they do not load the high-impedance ECG inputs. | VERIFY | `bom/preliminary_bom.md` |
| B7 | ECG connector type and mating | The Molex 5034801000 FFC/FPC connector (J3) is placed; confirm the mating part and electrode cable assembly before committing the connector footprint and keepout. | CONFIRM | `bom/preliminary_bom.md` |

## Group D — Safety interlock

| ID | Title | Why it blocks layout / what must be decided | Status | Reference |
|----|-------|---------------------------------------------|--------|-----------|
| B9 | USB_PRESENT detection | The interlock requires reliable detection of USB power presence; the sensing method and net must be defined before routing the interlock logic. | TBD | `Project_Specification.md` |
| B10 | AFE_ENABLE interlock (disable AFE/electrode path while on USB) | The mechanism that disables the AFE/electrode path on non-isolated USB must be designed so its control net and switching elements can be placed and routed. | TBD | `Project_Specification.md` |

## Group E — Power

| ID | Title | Why it blocks layout / what must be decided | Status | Reference |
|----|-------|---------------------------------------------|--------|-----------|
| B11 | Battery capacity + MP2662 charge current + NTC resistor | Battery selection sets the MP2662 (U3) charge-current programming resistor and the NTC thermistor network; these values must be set before the charger area is placed. | TBD | `bom/preliminary_bom.md` |
| B12 | microSD placement | The microSD socket (J2) location interacts with SPI routing and mechanical clearance; placement must be decided before layout. | TBD | `Project_Specification.md` |

## Group F — PCB / layout constraints

| ID | Title | Why it blocks layout / what must be decided | Status | Reference |
|----|-------|---------------------------------------------|--------|-----------|
| B13 | 4-layer PCB stackup | The preferred 4-layer stackup (top signal / GND / power+quiet-analog / bottom signal) and its impedance targets must be finalized to drive trace geometry. | FINALIZE | `Project_Specification.md` |
| B14 | ESP32-S3 antenna keepout | The WROOM-1 module antenna keepout is a fixed placement constraint that must be applied at layout to preserve RF performance. | CONSTRAINT | `Project_Specification.md` |
| B15 | Analog/digital routing constraints (separation, star return) | Analog/digital separation and star-return grounding rules must be applied during routing to protect the high-resolution ADC inputs. | CONSTRAINT | `ADS1298_Analog_Frontend_Notes.md` |

## Group G — Expansion

| ID | Title | Why it blocks layout / what must be decided | Status | Reference |
|----|-------|---------------------------------------------|--------|-----------|
| B16 | Expansion connector pinout (J5, 2x20) | Only the needed pins on the 2x20 header (J5) should be assigned; this should be refined after the multi-channel module architecture is defined so the pinout does not change post-layout. | REFINE | `Project_Specification.md` |

---

## Definition of done

PCB layout for CardioCore V1 may begin only when **all FIX, DECIDE, TBD, VERIFY/CONFIRM, FINALIZE, and REFINE items above are closed** (electrical values selected against verified datasheet parameters, parts confirmed, stackup and pinout pinned down) **and the complete schematic has been reviewed**. The **FIX** items — the two 🔴 critical defects (B17 CLKSEL, B18 RESV1 floating), B19, and B20 — and the **DECIDE** items (B8 montage, B22 coupling/supply scheme) are the **highest-priority prerequisites**: no other work should be locked while a FIX defect remains open, because it can invalidate downstream values. CONSTRAINT items (B14, B15) are not blockers to closing this list but **must be carried into and enforced during** the layout phase. Until then, no PCB layout, Gerber generation, or finalization of electrical values is to proceed.

## Research-use-only notice

CardioCore V1 and the CardioVest platform are for **research, prototyping, and education only**. This is **not a medical device** and is not intended for diagnosis, treatment, patient monitoring, emergency use, or clinical decision-making. No regulatory status (FDA, CE, ISO) is claimed or implied.
