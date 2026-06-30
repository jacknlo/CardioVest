# CardioCore V1 — flux.ai Change List

> **Apply these in flux.ai when access returns (~late July 2026).** This is the punch
> list captured from the AFE verification so the wait costs nothing and the next flux.ai
> session is fast. Source of detail: [`AFE_Verification_Report.md`](./AFE_Verification_Report.md).

> **Research / education only.** Not a medical device. Values below are **candidates to
> verify** against the ADS1298 datasheet (SBAS459) and the ADS1298ECG-FE reference design.

## 1. Critical — must fix before the board can work

- [ ] **F1 — CLKSEL strap.** Currently `CLKSEL → GND` (external-clock mode) with **no clock
  source** on the board → the ADS1298 will not clock. Re-strap **`CLKSEL → 3V3`** to use the
  internal oscillator (or add a 1–2.2 MHz clock source on `CLK`). *(Blocker B17)*

## 2. Decisions to settle before editing

- [ ] **F2 — Channel montage.** Inputs are wired as 5 **differential pairs**
  (RA-LA, LL-V1, V2-V3, V4-V5, V6-AUX), not WCT-referenced precordials. Decide: keep as-is
  (differential montage) **or** rewire precordials to the **Wilson Central Terminal (WCT)**.
  This changes input wiring, so settle it first. *(B8)*
- [ ] **F5 — IN5N "AUX".** Channel 5 negative input has **no electrode** (only D2.K5 / R18).
  Decide: keep as a documented spare (bias/terminate properly) or reassign to a real signal.

## 3. Component values to set (candidates — verify first)

- [ ] **Series input R (R9–R18):** `10 kΩ, 1 %` (matched P/N for CMRR). *(B2)*
- [ ] **Common-mode caps (C15–C24, to GND):** `470 pF, C0G/NP0`. *(B1)*
- [ ] **Differential caps (C25–C29, across pair):** `4.7 nF, C0G/NP0` (≈10× Ccm). *(B1)*
- [ ] **RLD:** `R20 (Rf) ≈ 1 MΩ`, `C30 (Cf) ≈ 1–2 nF` (stability — simulate), `R19 (series→RL)
  ≈ few kΩ–tens kΩ`, `C14 (RLDREF) ≈ 1 µF`. *(B3)*
- [ ] **Decoupling:** `C9 (VREFP) ≈ 1 µF` + bulk; `C10 (VCAP1) ≈ 1 µF` (verify 1 vs 10 µF);
  keep per-pin 100 nF; add bulk on 3V3 / USB_5V / VSYS. Differentiate the `C1–C14` group from
  a single 100 nF value. *(B5, F3)*

## 4. Confirmations (check, fix only if wrong)

- [ ] **F4 — VREFN → AVSS** (not found in netlist; confirm it ties to analog ground).
- [ ] **REF5025 (U5):** confirm VREFP headroom at AVDD = 3.3 V and that the ADS1298 internal
  reference buffer is disabled in firmware (`CONFIG3`). *(B4)*

## 5. After editing in flux.ai

- [ ] Re-export the **EDIF netlist** and **BOM**, replace the files under
  `hardware/CardioCore_V1/`, and re-run the AFE verification.
- [ ] Update [`Blockers_Before_PCB_Layout.md`](./Blockers_Before_PCB_Layout.md) as items close.
- [ ] Only then consider PCB layout (still gated by the blockers list).
