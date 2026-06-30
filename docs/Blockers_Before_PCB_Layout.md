# Blockers Before PCB Layout — CardioCore V1

This document is the authoritative, single-source list of everything that **must** be resolved before PCB layout begins for **CardioCore V1**, the 8-channel battery-powered ECG acquisition board in the CardioVest platform. Items here are derived from the analog-front-end review, the project specification, and the preliminary BOM. Each item carries an ID, a short rationale (why it blocks layout / what must be decided), a status, and where relevant a pointer to the supporting document.

> **PCB layout is BLOCKED until the ADS1298 analog front-end is verified and the items below are resolved.**

Status legend:

- **TBD** — value or design decision not yet made; depends on datasheet verification and schematic review.
- **VERIFY** — a part is placed, but a datasheet-level parameter must be confirmed before it is locked.
- **FINALIZE** — direction is set; the exact figures (stackup, impedance, pinout) must be pinned down.
- **CONSTRAINT** — a routing/placement rule to apply during layout (not a value to compute now).

Related documents:

- `docs/AFE_Verification_Report.md` — **verified netlist connectivity, channel map, findings (F1–F8), and candidate values** (start here).
- `hardware/CardioCore_V1/Project_Specification.md` — board targets, power workflow, safety interlock concept.
- `docs/ADS1298_Analog_Frontend_Notes.md` — AFE input network, decoupling, reference, RLD/DRL.
- `bom/preliminary_bom.md` — placed parts and reference designators.

---

## Group A — Analog front-end (ADS1298 input path)

| ID | Title | Why it blocks layout / what must be decided | Status | Reference |
|----|-------|---------------------------------------------|--------|-----------|
| B1 | ADS1298 input filter cap values (C15–C24 common-mode, C25–C29 differential) | Anti-alias / EMI filter caps set input bandwidth and CMRR balance at every channel; routing/footprints cannot be committed until values are chosen. Candidate values proposed in the AFE report. | TBD | `AFE_Verification_Report.md` |
| B2 | Series input resistor values (R9–R18) | Per-line series resistors form the input filter with B1 and limit fault current (note: **no** defibrillation protection is claimed/designed); value drives noise, footprint, and placement near the connector. | TBD | `AFE_Verification_Report.md` |
| B17 | ADS1298 clock strap (CLKSEL) | 🔴 **Critical:** CLKSEL is tied to **GND** (external-clock mode) but no clock source / CLK-pin connection exists — the device will not clock as wired. Re-strap CLKSEL **high** (internal oscillator) or add a clock source. | VERIFY | `AFE_Verification_Report.md` (F1) |
| B8 | Lead / channel mapping & montage (electrode → INxP/INxN, WCT) | Mapping is now **extracted** from the netlist (5 differential pairs: RA-LA, LL-V1, V2-V3, V4-V5, V6-AUX; IN6–8 on J5). **Decide** whether this differential montage is intended, or whether precordials should be WCT-referenced. | DECIDE | `AFE_Verification_Report.md` (F2) |

## Group B — Reference & decoupling

| ID | Title | Why it blocks layout / what must be decided | Status | Reference |
|----|-------|---------------------------------------------|--------|-----------|
| B4 | REF5025 reference implementation + decoupling | Decide standalone vs. buffered REF5025 (U5) and the associated capacitor values; this governs the reference node placement and the quiet-analog plane routing. | TBD | `ADS1298_Analog_Frontend_Notes.md` |
| B5 | ADS1298 decoupling (AVDD, AVSS, DVDD, VCAP1–4, VREFP, VREFN) | Decoupling values/placement around U2 are critical to ADC noise. **VREFP (C9) and VCAP1 (C10) need µF-range caps, not the 100 nF placeholder (F3)**; also confirm VREFN → AVSS (F4). | TBD | `AFE_Verification_Report.md` (F3, F4) |
| B3 | RLD/DRL network values and loop stability | The Right-Leg-Drive feedback network (Rf, Cf, output series R) and its loop stability define the common-mode rejection path; component values and the driven-electrode return route must be settled first. | TBD | `ADS1298_Analog_Frontend_Notes.md` |

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

PCB layout for CardioCore V1 may begin only when **all TBD, VERIFY/CONFIRM, FINALIZE, and REFINE items above are closed** (electrical values selected against verified datasheet parameters, parts confirmed, stackup and pinout pinned down) **and the complete schematic has been reviewed**. CONSTRAINT items (B14, B15) are not blockers to closing this list but **must be carried into and enforced during** the layout phase. Until then, no PCB layout, Gerber generation, or finalization of electrical values is to proceed.

## Research-use-only notice

CardioCore V1 and the CardioVest platform are for **research, prototyping, and education only**. This is **not a medical device** and is not intended for diagnosis, treatment, patient monitoring, emergency use, or clinical decision-making. No regulatory status (FDA, CE, ISO) is claimed or implied.
