# Blockers Before PCB Layout — CardioCore V1

This document is the authoritative, single-source list of everything that **must** be resolved before PCB layout begins for **CardioCore V1**, the 8-channel battery-powered ECG acquisition board in the CardioVest platform. Items here are derived from the analog-front-end review, the project specification, and the preliminary BOM. Each item carries an ID, a short rationale (why it blocks layout / what must be decided), a status, and where relevant a pointer to the supporting document.

> **PCB layout is BLOCKED until the ADS1298 analog front-end is verified and the items below are resolved.**

Status legend:

- **TBD** — value or design decision not yet made; depends on datasheet verification and schematic review.
- **VERIFY** — a part is placed, but a datasheet-level parameter must be confirmed before it is locked.
- **FINALIZE** — direction is set; the exact figures (stackup, impedance, pinout) must be pinned down.
- **CONSTRAINT** — a routing/placement rule to apply during layout (not a value to compute now).

Related documents:

- `hardware/CardioCore_V1/Project_Specification.md` — board targets, power workflow, safety interlock concept.
- `docs/ADS1298_Analog_Frontend_Notes.md` — AFE input network, decoupling, reference, RLD/DRL.
- `bom/preliminary_bom.md` — placed parts and reference designators.

---

## Group A — Analog front-end (ADS1298 input path)

| ID | Title | Why it blocks layout / what must be decided | Status | Reference |
|----|-------|---------------------------------------------|--------|-----------|
| B1 | ADS1298 input RC filter values (C15–C30) | Anti-alias / EMI filter capacitor values set the input bandwidth and impedance balance at every channel; routing and footprints cannot be committed until values and packages are chosen. | TBD | `ADS1298_Analog_Frontend_Notes.md` |
| B2 | Series protection resistor values (R9–R20) | The series input resistors define defibrillation/protection behaviour and combine with B1 to form the input filter; their value drives footprint, power rating, and placement near the connector. | TBD | `ADS1298_Analog_Frontend_Notes.md` |
| B8 | Lead / channel mapping (electrode → ADS1298 INxP/INxN, WCT handling) | Assignment of RA/LA/LL/RL-RLD/V1..V6 to specific INxP/INxN pins and the Wilson Central Terminal handling determine the entire analog routing topology; layout cannot begin without a fixed mapping. | TBD | `ADS1298_Analog_Frontend_Notes.md` |

## Group B — Reference & decoupling

| ID | Title | Why it blocks layout / what must be decided | Status | Reference |
|----|-------|---------------------------------------------|--------|-----------|
| B4 | REF5025 reference implementation + decoupling | Decide standalone vs. buffered REF5025 (U5) and the associated capacitor values; this governs the reference node placement and the quiet-analog plane routing. | TBD | `ADS1298_Analog_Frontend_Notes.md` |
| B5 | ADS1298 decoupling (AVDD, AVSS, DVDD, VCAP, VREFP, VREFN) | Decoupling capacitor values and their placement directly around U2 are critical to ADC noise performance and must be finalized before the component fan-out is placed. | TBD | `ADS1298_Analog_Frontend_Notes.md` |
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
