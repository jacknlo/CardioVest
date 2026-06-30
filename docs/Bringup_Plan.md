# CardioCore V1 — Board Bring-Up Plan

**Project:** CardioVest — wearable multi-channel ECG acquisition research platform
**Module:** CardioCore V1 (8-channel ECG acquisition board)
**Status:** Bring-up procedure for a freshly assembled prototype PCB.

> **RESEARCH / PROTOTYPING / EDUCATION ONLY.** CardioCore V1 is **not** a certified
> medical device. It must not be used for diagnosis, treatment, patient monitoring,
> emergency use, or any clinical decision-making. This document is an engineering
> bring-up procedure, not a clinical protocol.

---

## How to use this document

Bring-up is performed in **ordered stages**. Each stage has a **Goal**, **Steps**,
**Expected result**, and **Pass/Fail criteria**. **Do not advance to the next stage
until the current stage passes.** If a stage fails, stop, diagnose, and re-test the
failed stage before continuing — a failure early on (e.g. a power rail short) can
destroy parts populated in later stages.

### Reference hardware (for context)
- **MCU:** ESP32-S3-WROOM-1-N16R8 (16 MB flash / 8 MB PSRAM)
- **AFE:** Texas Instruments ADS1298 (8-channel, 24-bit, simultaneous-sampling)
- **Voltage reference:** REF5025 external 2.5 V precision reference
- **Power:** 1S LiPo battery, USB-C charging + programming
- **Connectivity:** BLE streaming, microSD logging
- **Electrode inputs:** RA, LA, LL, RL/RLD (right-leg drive), V1–V6

### Recommended equipment
- Bench DMM (with current range)
- Lab DC power supply with adjustable current limit (for safe first power-up)
- Oscilloscope (≥100 MHz, with probes; useful for SPI and rail ripple)
- USB-C cable + host PC with the project firmware toolchain
- Logic analyzer (optional but very helpful for SPI / Stage 3–4)
- ESD-safe workstation (mat, wrist strap)
- Thermal camera or finger-test discipline for spotting hot parts

### Safety reminder (read before Stage 8)
**Stage 8 and Stage 9 are the first body-connected tests.** From Stage 8 onward you
**must** follow `Safety_Research_Use.md`:
- **Battery power ONLY** while any electrode is connected to a person.
- **No USB cable connected** (no host PC, no charger, no other mains-referenced gear)
  while electrodes are on a body.
- Only informed adult research volunteers; participation is voluntary.
- This is signal-quality / engineering validation, **not** any form of medical assessment.

---

## Stage 0 — Visual Inspection

**Goal:** Confirm the assembled board is physically sound before any power is applied.

**Steps:**
1. Inspect the bare/assembled board under good light and magnification (10–20x).
2. Check every part for **correct orientation**: ESP32-S3 module pin 1, ADS1298 pin 1,
   REF5025, diodes, LEDs, electrolytic/tantalum caps, and the LiPo connector polarity.
3. Check for **solder defects**: bridges, cold joints, tombstoned passives, insufficient
   fillets, and lifted pads — pay special attention to the ADS1298 fine-pitch / thermal pad.
4. Check for **shorts** with a DMM in continuity mode:
   - VBAT to GND
   - +3.3 V (digital) to GND
   - Analog rails (e.g. AVDD / analog 3.3 V) to GND
   - REF5025 output to GND
5. Confirm decoupling capacitors are present at each supply pin and the reference.
6. Confirm the USB-C connector and microSD socket are seated and not bridged.

**Expected result:** No visible defects; **no continuity (no short)** between any
supply net and GND.

**Pass/Fail:**
- **PASS:** All parts correct orientation, no solder bridges/shorts, no rail-to-GND short.
- **FAIL:** Any rail shows a dead short to GND, any major part is reversed, or any
  fine-pitch joint is suspect. Rework before applying power.

---

## Stage 1 — Power Rails

**Goal:** Verify every supply rail is present, correct, and stable **before** trusting
sensitive parts. Where feasible, validate rails with sensitive ICs unpopulated or before
enabling their supplies.

**Steps:**
1. **First power-up on a current-limited bench supply**, not the battery:
   - Set supply to the nominal 1S LiPo voltage (TBD nominal, e.g. ~3.7–4.0 V).
   - Set a conservative current limit (TBD, start low, e.g. ~100–150 mA) to catch shorts.
2. Apply power and immediately observe input current. It should be small and stable —
   not pinned at the current limit.
3. Measure rails with the DMM:
   - **+3.3 V digital rail** — within tolerance (TBD target, e.g. 3.3 V ±5%).
   - **Analog supply rail(s)** for the ADS1298 (AVDD and any analog 3.3 V) — within tolerance.
   - **REF5025 output** — should read **2.500 V** (within REF5025 datasheet accuracy).
4. Check rail **ripple/noise** on the scope (AC-coupled). Analog rail and reference should
   be quiet (TBD ripple budget).
5. **Charger check (USB-C):**
   - Connect USB-C with the battery (or a battery emulator) attached.
   - Confirm the charge IC enters charge state, charge LED behaves as designed, and VBAT
     trends upward. Confirm charge current is within the configured limit (TBD).
   - Confirm no rail exceeds its maximum when both USB and battery are present.
6. Spot-check part temperatures — nothing should be hot.

**Expected result:** All rails at target voltage, low ripple; REF5025 at 2.5 V; charger
charges the battery safely; total current draw is sane.

**Pass/Fail:**
- **PASS:** Every rail within tolerance, REF5025 ≈ 2.500 V, charger functions, no overheating.
- **FAIL:** Any rail out of tolerance, excessive current, reference wrong, or charger
  misbehaving. **Do not proceed** — a bad rail can damage the ESP32-S3 or ADS1298.

---

## Stage 2 — ESP32-S3 Boot

**Goal:** Confirm the MCU programs over USB-C and runs firmware, producing a serial banner.

**Steps:**
1. Connect the board to the host PC via USB-C.
2. Confirm the USB-serial interface enumerates (native USB CDC on the ESP32-S3, or the
   onboard USB-UART bridge, per the board design — TBD which is fitted).
3. Flash a minimal bring-up firmware (blink + serial banner + chip info).
4. Open a serial terminal at the project baud rate (TBD, e.g. 115200).
5. Reset the board and observe the startup banner: firmware name/version, chip model,
   flash size (expect 16 MB), and PSRAM detection (expect 8 MB).
6. Confirm PSRAM initializes (the N16R8 part has 8 MB octal PSRAM — firmware should report it).

**Expected result:** Board enumerates, accepts a flash, reboots, and prints a clean
startup banner. Flash and PSRAM are detected at expected sizes.

**Pass/Fail:**
- **PASS:** Firmware flashes successfully; banner prints; 16 MB flash and 8 MB PSRAM detected.
- **FAIL:** Won't enter download mode, won't enumerate, flash/PSRAM mis-detected, or
  resets/brownouts during boot. Check strapping pins, USB lines, and the +3.3 V rail.

---

## Stage 3 — SPI Bus Test

**Goal:** Verify the SPI bus the MCU uses to talk to the ADS1298 (and microSD) is wired
and toggling correctly **before** trusting register transactions.

**Steps:**
1. From firmware, configure the SPI peripheral (SCLK, MOSI, MISO, CS) at a low test clock.
2. Drive a known pattern out on MOSI and toggle SCLK and the ADS1298 CS line.
3. With a scope or logic analyzer, confirm:
   - SCLK toggles at the expected frequency with clean edges.
   - MOSI carries the expected pattern.
   - CS asserts/deasserts around transactions.
   - MISO is idle/high-Z as expected when no slave drives it.
4. **Loopback check (optional, recommended):** temporarily jumper MOSI→MISO and confirm
   the MCU reads back what it transmits, proving the controller path end-to-end.
5. Confirm the ADS1298 control lines are reachable: `START`, `RESET`, `PWDN`, `DRDY`, `CS`.

**Expected result:** Clean SPI clock and data on the bus; loopback (if used) returns the
transmitted bytes; ADS1298 control/status lines are present and controllable.

**Pass/Fail:**
- **PASS:** SPI signals clean and at expected timing; loopback matches; control lines wired.
- **FAIL:** Missing/garbled clock or data, no CS activity, or loopback mismatch.
  Fix wiring/pin-mux/level issues before talking to the ADS1298.

---

## Stage 4 — ADS1298 Register Read

**Goal:** Establish real bidirectional communication with the ADS1298 by reading known
register values.

**Steps:**
1. Power/enable the ADS1298 analog and digital supplies; release `PWDN` and pulse `RESET`
   per the datasheet power-up sequence.
2. Confirm the ADS1298 clock source is running (internal oscillator or external clock — TBD).
3. Issue the SPI command to stop continuous read mode (`SDATAC`) before register access.
4. Read the **ID register** and confirm it matches the expected ADS1298 device ID
   (decode device family and channel count fields per the datasheet).
5. Read a few additional registers with known reset defaults (e.g. `CONFIG1`–`CONFIG3`)
   and verify they match datasheet reset values.
6. Write a register, read it back, and confirm the written value sticks (proves write path).

**Expected result:** ID register reports an ADS1298; default registers match datasheet
reset values; write-then-read confirms the write path.

**Pass/Fail:**
- **PASS:** Device ID correct, default registers correct, write/read-back consistent.
- **FAIL:** ID wrong/all-0x00/all-0xFF, defaults inconsistent, or writes don't stick.
  Re-check SPI mode/CPOL/CPHA, CS, the power-up/reset sequence, and the clock source.

---

## Stage 5 — Internal Test Signal

**Goal:** Validate the **acquisition chain** (AFE → SPI → MCU) end-to-end using the
ADS1298 internal square-wave test signal, **without any electrodes**.

**Steps:**
1. Configure the ADS1298 to route the **internal test signal** to the channel inputs
   (set the test-signal enable/amplitude/frequency bits in `CONFIG2`, and select the
   test source in each channel's `CHnSET`).
2. Configure a known data rate (TBD default sample rate) and PGA gain.
3. Issue `RDATAC` (continuous read) and `START`; service the `DRDY` interrupt.
4. Stream samples to the host (over the USB serial link at this stage).
5. Plot/inspect the data: every enabled channel should show the expected **square wave**
   at the configured amplitude and frequency, with the correct sign and 24-bit scaling.
6. Sanity-check the status word and channel ordering in each `DRDY` frame.

**Expected result:** All channels output a clean, correctly-scaled internal square wave;
data frames decode correctly with no dropped/garbled samples.

**Pass/Fail:**
- **PASS:** Square wave present on all enabled channels at expected amplitude/frequency;
  frame parsing and scaling correct; no sample loss.
- **FAIL:** Flat/noisy/clipped channels, wrong amplitude or frequency, mis-ordered
  channels, or dropped frames. Debug AFE config, `DRDY` handling, and frame parsing.

---

## Stage 6 — microSD Logging

**Goal:** Confirm the microSD card mounts and the firmware can reliably write and read
back a test log (needed for later high-rate acquisition logging).

**Steps:**
1. Insert a known-good, formatted microSD card (FAT32 recommended; TBD card class/size).
2. Initialize the SD interface (SPI mode or SDMMC, per the board design — TBD) and mount
   the filesystem.
3. Print card info (type, capacity, filesystem) to serial.
4. Write a test file with a known payload (e.g. a counter or a fixed text block).
5. Close, re-open, read the file back, and byte-compare against what was written.
6. Run a short **throughput test** writing a continuous stream to estimate sustained
   write speed (relevant for full-rate 8-channel logging — TBD target throughput).

**Expected result:** Card mounts; written and read-back data match exactly; sustained
write throughput is sufficient for the intended acquisition rate.

**Pass/Fail:**
- **PASS:** Card mounts, read-back matches write, throughput meets the (TBD) budget.
- **FAIL:** Mount fails, data mismatch/corruption, or throughput too low. Check SD wiring,
  pull-ups, interface mode, supply stability, and card compatibility.

---

## Stage 7 — BLE Streaming

**Goal:** Confirm BLE advertises, accepts a connection, and streams test data to a host
**without** a body connection.

**Steps:**
1. Flash/enable firmware with the BLE GATT service for sample streaming (TBD service/
   characteristic UUIDs and packet format).
2. Confirm the board advertises with the expected device name.
3. Connect from a host (phone app or PC BLE tool); confirm pairing/connection succeeds.
4. Stream **synthetic or internal-test-signal** data (from Stage 5) over a notify/indicate
   characteristic.
5. On the host, verify packet integrity: sequence numbers continuous, no dropped/duplicated
   packets, correct decoding back to sample values.
6. Note throughput, connection interval/MTU, and stability over several minutes.

**Expected result:** Stable BLE advertise → connect → stream; host reconstructs the test
data correctly at an acceptable, stable rate.

**Pass/Fail:**
- **PASS:** Advertises, connects reliably, streams with intact sequencing and acceptable
  throughput over a sustained run.
- **FAIL:** Won't advertise/connect, drops connection, or loses/corrupts packets. Debug
  BLE stack config, MTU/connection params, and the packetizer.

---

## Stage 8 — Battery-Only ECG Test (FIRST body-connected test)

> **SAFETY GATE — follow `Safety_Research_Use.md`.**
> This is the **first** test with electrodes on a person.
> - **Battery power ONLY.**
> - **USB-C disconnected** — no host PC, no charger, nothing mains-referenced attached
>   to the board while electrodes are on a body.
> - Informed, voluntary adult research volunteer only.
> - Engineering signal-quality check **only** — not a medical assessment, no diagnosis,
>   no interpretation of the trace.

**Goal:** Acquire a real biopotential signal on a small electrode set, on battery power,
to validate the analog front-end and electrode interface in the real world.

**Steps:**
1. Confirm the board is running on the **battery** and **USB is unplugged**. Verify
   adequate battery charge before starting.
2. Connect a reduced electrode set first (e.g. RA, LA, LL, and RL/RLD right-leg drive)
   using fresh disposable electrodes and the project lead wires.
3. Enable the **right-leg drive (RLD)** path and configure the relevant channels and gain.
4. Record a short capture (logged to microSD and/or streamed over BLE — **not** USB).
5. Inspect the captured waveform for a plausible, periodic biopotential signal and assess
   noise, baseline wander, and 50/60 Hz mains pickup.
6. Compare RLD on vs. off to confirm the drive reduces common-mode interference.

**Expected result:** A clean, periodic biopotential waveform on battery power, with RLD
visibly improving common-mode rejection; acceptable noise floor and baseline behavior.

**Pass/Fail:**
- **PASS:** Recognizable, stable periodic signal; RLD reduces interference; data captured
  via microSD/BLE with the board fully battery-isolated from mains.
- **FAIL:** No usable signal, excessive mains/motion noise that RLD can't tame, saturated
  channels, or any safety-procedure deviation. Stop and resolve before continuing.

---

## Stage 9 — Full 8-Channel Acquisition

> **SAFETY GATE — continues under `Safety_Research_Use.md`:** battery-only, USB
> disconnected while electrodes are on a body. Same volunteer/consent conditions as Stage 8.

**Goal:** Acquire all 8 channels simultaneously from the full electrode set and
sanity-check data integrity across the whole acquisition chain.

**Steps:**
1. On battery power (USB unplugged), connect the full electrode set: RA, LA, LL, RL/RLD,
   and V1–V6.
2. Enable all 8 ADS1298 channels in simultaneous-sampling mode at the target sample rate
   and gain (TBD final defaults).
3. Capture a multi-channel record to microSD and/or BLE.
4. **Sanity-check data integrity:**
   - All 8 channels carry plausible, live data (no dead/flat/stuck channels).
   - Channels are correctly mapped to their electrode labels (no swaps).
   - Sample timestamps/sequence are continuous — **no dropped samples** at full rate.
   - Inter-channel timing is consistent with simultaneous sampling.
   - No saturation/clipping at the configured gain.
5. Verify the logging/streaming path sustains the full 8-channel data rate without loss
   (cross-checks the Stage 6 throughput budget under real load).
6. Record observations on overall signal quality and per-channel noise.

**Expected result:** Eight simultaneously-sampled channels of plausible biopotential data,
correctly labeled, with no dropped samples, captured on battery power within the logging/
streaming throughput budget.

**Pass/Fail:**
- **PASS:** All 8 channels live and correctly mapped, simultaneous timing intact, zero
  sample loss at full rate, no clipping, sustained capture on battery only.
- **FAIL:** Any dead/swapped channel, dropped samples, throughput overrun, clipping, or
  safety-procedure deviation. Resolve before declaring CardioCore V1 bring-up complete.

---

## Bring-up completion checklist

- [ ] Stage 0 — Visual inspection passed
- [ ] Stage 1 — Power rails passed (incl. REF5025 = 2.5 V, charger)
- [ ] Stage 2 — ESP32-S3 boots, flash/PSRAM detected
- [ ] Stage 3 — SPI bus verified
- [ ] Stage 4 — ADS1298 register read/write verified
- [ ] Stage 5 — Internal test signal across acquisition chain
- [ ] Stage 6 — microSD log write/read verified
- [ ] Stage 7 — BLE advertise/connect/stream verified
- [ ] Stage 8 — Battery-only first body-connected test (per `Safety_Research_Use.md`)
- [ ] Stage 9 — Full 8-channel simultaneous acquisition, data integrity verified

**Reminder:** Stages 8–9 are body-connected and **must** run battery-only with USB
disconnected, per `Safety_Research_Use.md`. CardioCore V1 remains a research/education
prototype and must never be used for diagnosis, treatment, or any clinical purpose.
