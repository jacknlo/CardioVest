# CardioCore V1 — TODO

> PCB layout is blocked — see `docs/Blockers_Before_PCB_Layout.md`.

Short, actionable next tasks for CardioCore V1. Schematic/EDA tool is **flux.ai**.
Research / prototyping / education only — not a medical device.

## Analog front-end (highest priority / blocking)

- [ ] Verify ADS1298 input RC filter values from datasheet (C15–C30 + R9–R20) and set them (B1, B2)
- [ ] Verify ADS1298 decoupling from datasheet: AVDD, AVSS, DVDD, **VCAP**, VREFP, VREFN cap values (B5)
- [ ] Verify/define RLD/DRL circuit: Rf, Cf, output series R, and loop stability (B3)
- [ ] Verify REF5025 reference implementation + decoupling (standalone vs buffered, cap values) (B4)
- [ ] Select/confirm low-leakage ESD parts: verify PESD3V3L5UY reverse-leakage + capacitance (B6)
- [ ] Define lead/channel mapping (electrode → ADS1298 INxP/INxN, WCT handling) (B8)
- [ ] Fill ADS1298 pin-audit "Connected To" column from the EDIF netlist (`schematics/CardioCore_V1.edif`)

## Safety interlock

- [ ] Define USB_PRESENT detection circuit (B9)
- [ ] Define AFE_ENABLE interlock that disables the AFE/electrode path while on USB (B10)

## Power

- [ ] Define ECG connector mating + electrode cable for Molex 5034801000 FFC/FPC (B7)
- [ ] Set battery capacity, MP2662 charge current, and NTC resistor values (B11)

## Firmware

- [ ] Build the ESP32-S3 firmware skeleton (boot, SPI to ADS1298, USB enumeration)
- [ ] Wire AFE_ENABLE / USB_PRESENT GPIO handling into the firmware interlock logic

## Software

- [ ] Create a dummy-data ECG viewer (multi-channel plot, no hardware required)

## Repo / docs

- [ ] Prepare the flux.ai schematic review (ERC pass, net naming, design-block check)
- [ ] Finalize 4-layer stackup + impedance, ESP32-S3 antenna keepout, and analog/digital routing constraints (B13, B14, B15)
- [ ] Refine expansion connector pinout (J5 2x20) after the multi-channel module architecture is defined (B16)
