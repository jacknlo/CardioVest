# ADS1298 Analog Front-End — Engineering Notes (CardioCore V1)

> **Status: WORKING NOTES.** These are pre-layout engineering notes for the
> CardioCore V1 analog front-end (AFE) built around the Texas Instruments
> **ADS1298** (8-channel, 24-bit, simultaneous-sampling ECG/EEG AFE). Many
> component values and topology choices are deliberately marked **TBD** and
> **must be finalized, reviewed, and verified against the ADS1298 datasheet and
> a SPICE/bench check before PCB layout is committed.** Do not treat any value
> here as final.
>
> **Scope / safety:** CardioCore V1 is a **research, prototyping, and education**
> platform. It is **not** a certified medical device and is not intended for
> diagnosis, treatment, patient monitoring, emergency use, or clinical
> decision-making. These notes cover signal-integrity and electrical-design
> concerns only. Any human-subject use must go through the appropriate ethics /
> safety review and isolation provisions, which are out of scope for this file.

---

## 1. ECG input network (TBD)

The input network sits between each electrode connector (RA, LA, LL, RL/RLD,
V1–V6) and the ADS1298 differential input pins (INxP / INxN). Design decisions
still open:

- **Input coupling (AC vs. DC):** TBD.
  - DC coupling preserves the baseline and lets the ADS1298's internal
    high-pass options / digital filtering handle drift, but requires the input
    common-mode to stay within the AFE's linear range and is sensitive to large
    electrode half-cell offsets (±300 mV typical, can be larger).
  - AC coupling (series cap + bias resistor) removes electrode offset before the
    PGA but adds a passive high-pass corner, large-value caps, and settling-time
    considerations. Decide per the intended research bandwidth (e.g. diagnostic
    ~0.05–150 Hz vs. monitoring ~0.5–40 Hz). **Bandwidth target: TBD.**
- **Common-mode operating point:** TBD. Confirm how the patient/subject
  common-mode is established (via RLD, see §5) and that INxP/INxN stay inside
  the ADS1298 input common-mode range for the chosen supply scheme
  (unipolar vs. bipolar analog supply — **supply scheme: TBD**).
- **Source impedance / input bias:** electrodes are a **high-impedance** source
  (skin–electrode impedance can be hundreds of kΩ, especially dry electrodes).
  The input network plus any bias resistors must not load this down enough to
  degrade CMRR or attenuate the signal. **Bias resistor values: TBD.**
- **Channel mapping:** confirm the lead-to-INx assignment (Wilson Central
  Terminal handling for V1–V6, limb leads, derived leads) — whether done in
  hardware or computed in firmware from raw channels. **Decision: TBD.**

---

## 2. Series protection resistors (TBD)

Each electrode line should have a **series resistor** on the subject side,
before the RC filter and the ADS1298 pin.

- **Purpose:**
  - **Current limiting** — bound the worst-case current that could flow into or
    out of an electrode under fault (e.g. an input pin clamped to a rail, an ESD
    event, or accidental contact with another potential). This is a
    subject-side protection measure.
  - **Defibrillator / large-transient survivability** is a *separate* topic and
    is **explicitly out of scope** for V1 (no defib protection is claimed or
    designed here).
  - Forms the **R** of the input RC filter (§3) and damps interaction with the
    high source impedance and ESD device capacitance.
- **To determine (TBD):**
  - Resistance value per line (typ. range for ECG AFEs is tens of kΩ; **exact
    value TBD**) balancing fault current limit against noise (Johnson noise
    `√(4kTR·BW)`) and against the RC corner.
  - Matching tolerance between the P and N lines of a pair (mismatch degrades
    CMRR). **Tolerance: TBD (1% candidate, verify).**
  - Power rating / pulse rating and physical size.
  - Placement (as close to the connector as practical).

---

## 3. RC filter values (TBD)

A passive **RC low-pass** per input line provides anti-alias attenuation and
**EMI/RF input filtering** (rejecting RF that would otherwise rectify in the PGA
and appear as offset/baseline error).

- **Considerations / what to size:**
  - **Anti-alias:** the ADS1298 has a built-in (sinc) digital decimation filter,
    so the external RC is mainly for out-of-band/RF energy and aliasing of
    high-frequency interference, not the primary anti-alias filter. Corner must
    sit well above the ECG band of interest but below the modulator/RF region.
    **Corner frequency: TBD** (depends on chosen sample rate and bandwidth).
  - **Differential vs. common-mode filtering:** size separate differential-mode
    and common-mode (line-to-ground) corners. A common approach is per-line R +
    line-to-GND cap (Ccm) plus a differential cap (Cdiff) across the pair, with
    `Cdiff ≈ 10× Ccm` so the differential corner is dominated by the larger cap
    and small Ccm mismatch has less CMRR impact. **Ccm / Cdiff: TBD.**
  - **RC matching → CMRR:** mismatch in R or Ccm converts common-mode to
    differential. Use matched values / tight tolerance. **Tolerances: TBD.**
  - **NP0/C0G dielectric** for the filter caps (low voltage coefficient, low
    distortion). **Confirmed dielectric: TBD.**
  - Interaction with series protection R (§2) — these share the resistor.

---

## 4. Low-leakage ESD requirements

ECG inputs are **high-impedance** nodes. Any leakage path to a rail or ground
develops an error voltage across the large source + series-protection impedance,
producing **DC offset, baseline shift, and per-channel drift** — and any
*mismatch* in leakage between P/N legs degrades CMRR.

- Therefore ESD/TVS protection on INxP/INxN must be **low-leakage** (nanoampere
  or sub-nA reverse leakage) parts, not general-purpose diodes. A standard
  TVS/Zener with µA-scale leakage across tens of kΩ can create millivolts of
  offset and visible thermal drift.
- **Low capacitance** is also desirable: ESD device capacitance combines with
  the series R to form an unintended pole and can mismatch between legs.
- Select for: low reverse leakage (specified vs. temperature), low junction
  capacitance, appropriate clamp voltage relative to the analog supply, and
  matched P/N behavior. **Specific part: TBD.**
- Keep ESD parts close to the connector, ahead of the series R where possible so
  the resistor limits clamp current.

---

## 5. RLD / DRL (Right-Leg Drive) circuit notes

- **Purpose:** the Right-Leg Drive (a.k.a. Driven-Right-Leg, DRL) actively
  drives the subject's body common-mode to improve **common-mode rejection** —
  primarily knocking down 50/60 Hz mains pickup — by forming a feedback loop
  that injects an inverted, amplified version of the sensed common-mode back
  into the subject (via the RL electrode), rather than relying on passive
  grounding.
- **ADS1298 RLD amplifier:** the ADS1298 integrates an RLD amplifier with
  configurable channel selection. The common-mode is sensed from a selectable
  set of input channels (set via `RLD_SENSP` / `RLD_SENSN` registers), the RLD
  amp output appears on **RLDOUT**, and **RLDIN** provides the feedback path.
  Register configuration (which channels contribute to the average, internal vs.
  external RLD reference, lead-off interaction) is **TBD in firmware**.
- **Feedback network considerations (TBD):**
  - External **Rf / Cf** around the RLD amp set loop gain and the
    high-frequency rolloff for **stability** — too much gain or bandwidth can
    make the loop oscillate. **Rf, Cf: TBD; verify stability.**
  - A **series resistor on the RL/RLD output line to the subject** limits the
    maximum current that the drive can source/sink (safety + fault limiting).
    **Value: TBD.**
  - Decide RLD reference point (mid-supply / internal reference) and how it ties
    to the chosen common-mode operating point in §1. **TBD.**
  - Lead-off / electrode-disconnect behavior of the RLD loop. **TBD.**

---

## 6. REF5025 external 2.5 V reference notes

CardioCore V1 uses the **REF5025** (2.5 V precision reference) as the external
reference for the ADS1298 (rather than the internal reference) for lower noise /
better drift, feeding **VREFP** (with **VREFN** = analog ground / AVSS per
datasheet).

- **Wiring:** REF5025 output → VREFP; VREFN to the analog reference ground.
  Confirm the ADS1298 is configured for **external reference** (internal ref
  buffer disabled via `CONFIG3`). **Firmware config: TBD/verify.**
- **Filtering / buffering:**
  - REF5025 requires its specified output capacitor; follow the datasheet for
    stable operation and to avoid the cap-induced instability window.
  - The ADS1298 VREFP pin draws **dynamic (switched) current** from the
    sampling network. Provide adequate local bulk + HF decoupling at VREFP so
    these current pulses don't modulate the reference voltage (reference noise
    maps directly into measurement noise/INL). Consider whether an additional
    low-noise **buffer** between REF5025 and VREFP is warranted, or whether the
    REF5025 + local caps suffice. **Decision + cap values: TBD.**
- **Layout proximity:** place REF5025 and its output/decoupling caps **close to
  the ADS1298 VREFP pin** with a short, low-impedance, low-loop-area path on the
  analog ground. Keep away from digital switching nodes.
- **Noise:** REF noise is in-band; if a noise-reduction filter is used, account
  for its settling time. **Filter: TBD.**

---

## 7. Decoupling requirements

Goal: stable, quiet rails and clean **analog/digital separation**. Provide
local decoupling at every supply pin; values below are **starting candidates,
all TBD** until verified against datasheet recommendations and bench results.

- **ADS1298 analog (AVDD / AVSS):** per-pin local HF ceramic (e.g. 100 nF) plus
  shared bulk (e.g. 1–10 µF). NP0/C0G or X7R as appropriate. **TBD.**
- **ADS1298 digital (DVDD / DGND):** per-pin local HF ceramic (e.g. 100 nF)
  plus bulk. **TBD.**
- **Reference (VREFP / REF5025):** dedicated local decoupling as in §6. **TBD.**
- **ESP32-S3 MCU rails:** per-datasheet decoupling on 3V3 and any module pins;
  bulk near the module; extra attention near the radio supply. **TBD.**
- **Analog/digital separation:**
  - Keep analog and digital supplies and grounds partitioned; decide on a
    **single-point / star tie** strategy (where AGND meets DGND) — **TBD.**
  - Route SPI/clock (digital, fast edges) away from the high-impedance analog
    input and reference nodes.
  - Confirm whether a ferrite/filtered feed is used between the digital and
    analog supply domains. **TBD.**
- Place all decoupling caps with the **shortest loop area** to the respective
  pin and ground.

---

## 8. ADS1298 pin audit placeholder

Working audit of the key ADS1298 signals. **All connections are candidates and
every Status is TBD** until schematic capture is reviewed against the datasheet
and the CardioCore V1 net list. (Exact pin numbers depend on the chosen package
— **package: TBD** — so this table is organized by signal, not pin number.)

| Pin / Signal      | Function                                              | Connected To                             | Status |
|-------------------|------------------------------------------------------|------------------------------------------|--------|
| AVDD              | Analog positive supply                               | Analog rail + local decoupling           | TBD    |
| AVSS              | Analog supply return / negative analog rail          | Analog ground (AGND)                      | TBD    |
| DVDD              | Digital supply                                        | Digital rail (1.8/3.3 V — verify)        | TBD    |
| DGND              | Digital ground                                         | Digital ground (DGND)                     | TBD    |
| VREFP             | Positive reference input/output                       | REF5025 2.5 V out + local decoupling      | TBD    |
| VREFN             | Negative reference                                     | Analog reference ground (AVSS)            | TBD    |
| CLK               | Master clock input                                     | Clock source (ext osc vs. ESP32 — TBD)    | TBD    |
| SCLK              | SPI clock                                              | ESP32-S3 SPI SCLK                         | TBD    |
| DIN               | SPI data in (MOSI)                                     | ESP32-S3 SPI MOSI                         | TBD    |
| DOUT              | SPI data out (MISO)                                    | ESP32-S3 SPI MISO                         | TBD    |
| CS                | SPI chip select (active low)                           | ESP32-S3 GPIO (CS)                        | TBD    |
| DRDY              | Data-ready (active low, interrupt)                     | ESP32-S3 GPIO (IRQ)                       | TBD    |
| START             | Conversion start control                              | ESP32-S3 GPIO (or tied — TBD)             | TBD    |
| RESET             | Device reset (active low)                              | ESP32-S3 GPIO                             | TBD    |
| PWDN              | Power-down (active low)                                | ESP32-S3 GPIO (or tied high — TBD)        | TBD    |
| IN1P              | Channel 1 positive input                              | Electrode input network (lead TBD)        | TBD    |
| IN1N              | Channel 1 negative input                              | Electrode input network (lead TBD)        | TBD    |
| IN2P              | Channel 2 positive input                              | Electrode input network (lead TBD)        | TBD    |
| IN2N              | Channel 2 negative input                              | Electrode input network (lead TBD)        | TBD    |
| IN3P              | Channel 3 positive input                              | Electrode input network (lead TBD)        | TBD    |
| IN3N              | Channel 3 negative input                              | Electrode input network (lead TBD)        | TBD    |
| IN4P              | Channel 4 positive input                              | Electrode input network (lead TBD)        | TBD    |
| IN4N              | Channel 4 negative input                              | Electrode input network (lead TBD)        | TBD    |
| IN5P              | Channel 5 positive input                              | Electrode input network (lead TBD)        | TBD    |
| IN5N              | Channel 5 negative input                              | Electrode input network (lead TBD)        | TBD    |
| IN6P              | Channel 6 positive input                              | Electrode input network (lead TBD)        | TBD    |
| IN6N              | Channel 6 negative input                              | Electrode input network (lead TBD)        | TBD    |
| IN7P              | Channel 7 positive input                              | Electrode input network (lead TBD)        | TBD    |
| IN7N              | Channel 7 negative input                              | Electrode input network (lead TBD)        | TBD    |
| IN8P              | Channel 8 positive input                              | Electrode input network (lead TBD)        | TBD    |
| IN8N              | Channel 8 negative input                              | Electrode input network (lead TBD)        | TBD    |
| RLDOUT            | Right-leg drive amplifier output                      | RLD feedback network → RL electrode       | TBD    |
| RLDIN             | Right-leg drive feedback input                        | RLD feedback network                      | TBD    |

> Additional signals to capture before layout (also TBD): RLDREF, the WCT pins,
> GPIO1–GPIO4, DAISY_IN/cascade pins, CLKSEL, and any test/no-connect pins —
> resolve all against the datasheet for the chosen package.

---

## 9. Open questions before PCB layout

- Analog supply scheme: **unipolar vs. bipolar** AVDD/AVSS, and resulting input
  common-mode range?
- Input coupling: **DC vs. AC**, and the target ECG **bandwidth / sample rate**?
- Final **series protection resistor** value, tolerance, and pulse/power rating?
- Final **RC filter** corner(s) — differential and common-mode caps, dielectric,
  and matching tolerance?
- Selected **low-leakage, low-capacitance ESD/TVS** part for the inputs?
- **RLD** feedback network (Rf, Cf), output series resistor, sensed-channel set,
  and **loop stability** verification?
- REF5025: standalone or **buffered**? Output/decoupling cap values? Need a
  noise-reduction filter?
- **Clock source:** external oscillator vs. MCU-provided clock, and its noise
  impact on the AFE?
- **Grounding strategy:** AGND/DGND partition and single-point tie location;
  ferrite/filter between domains?
- ADS1298 **package** selection (drives pinout, footprint, thermal)?
- Lead/derived-lead computation (WCT handling): **hardware vs. firmware**?
- Lead-off detection scheme and its interaction with the input network and RLD?

---

*End of working notes — all TBD items must be finalized and reviewed before PCB
layout is committed.*
