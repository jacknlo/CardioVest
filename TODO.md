# CardioCore V1 — TODO

> PCB layout is blocked — see `docs/Blockers_Before_PCB_Layout.md`.

Short, actionable next tasks for CardioCore V1. Schematic/EDA tool is **flux.ai**.
Research / prototyping / education only — not a medical device.

## Analog front-end (highest priority / blocking)

- [x] Extract ADS1298 connectivity + channel map from the EDIF netlist → see `docs/AFE_Verification_Report.md`
- [ ] 🔴 Fix CLKSEL strap: tied to GND (external clock) with no clock source — re-strap high (internal osc) or add a clock (B17, F1)
- [ ] Decide channel montage: keep the 5 differential pairs as wired, or reference precordials to WCT (B8, F2)
- [ ] Set ADS1298 input filter values: R9–R18 (series), C15–C24 (common-mode), C25–C29 (differential) — candidates in the AFE report (B1, B2)
- [ ] Fix decoupling: VREFP (C9) + VCAP1 (C10) need µF-range (not 100 nF); confirm VREFN → AVSS (B5, F3, F4)
- [ ] Verify/define RLD/DRL: R20 (Rf), C30 (Cf), R19 (series to RL), C14 (RLDREF) + loop stability (B3)
- [ ] Verify REF5025 implementation + decoupling (buffer? cap values; headroom on 3.3 V) (B4)
- [ ] Verify PESD3V3L5UY (D1/D2) reverse-leakage + capacitance for high-Z inputs (B6)
- [ ] Decide IN5N "AUX" input: documented spare vs reassign; terminate/bias properly (F5)
- [ ] File findings F1–F8 as GitHub issues using the hardware_review template

## Safety interlock

- [ ] Define USB_PRESENT detection circuit (B9)
- [ ] Define AFE_ENABLE interlock that disables the AFE/electrode path while on USB (B10)

## Power

- [ ] Define ECG connector mating + electrode cable for Molex 5034801000 FFC/FPC (B7)
- [ ] Set battery capacity, MP2662 charge current, and NTC resistor values (B11)

## Firmware

- [x] Build the ESP32-S3 firmware skeleton (PlatformIO; ADS1298/ADS1292R, BLE, SD, interlock, demo stream) — compiles clean
- [ ] Add a symptom-marker button (SW1/SW2) that timestamps the data stream (Zio-style research event marker)
- [ ] Wire AFE_ENABLE / USB_PRESENT GPIO handling into the firmware interlock logic

## Software

- [x] Create an ECG viewer (web / Web Bluetooth, with synthetic demo) → `software/web_viewer/`
- [ ] Build a signal-quality index (SQI): classify each segment good vs noise — *classify, don't delete* (first DSP brick)
- [ ] Experiment with gentle, morphology-preserving filtering (baseline-wander HP, 50/60 Hz notch) — research only
- [ ] Mobile app: on-device voice/event annotations (speech→text, text+timestamp, synced to ECG via `cardiovest.annotations.v1`) — see `software/mobile_app/app_plan.md` §5.5
- [ ] Show annotation markers on the ECG viewer timeline (jump-to-event)

## Repo / docs

- [ ] Prepare the flux.ai schematic review (ERC pass, net naming, design-block check)
- [ ] Finalize 4-layer stackup + impedance, ESP32-S3 antenna keepout, and analog/digital routing constraints (B13, B14, B15)
- [ ] Refine expansion connector pinout (J5 2x20) after the multi-channel module architecture is defined (B16)
