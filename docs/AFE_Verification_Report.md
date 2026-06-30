# CardioCore V1 — Analog Front-End Verification Report

> **Status: REVIEW — candidate values, not finalized.** This report is generated from
> the committed flux.ai EDIF netlist
> ([`../hardware/CardioCore_V1/schematics/CardioCore_V1.edif`](../hardware/CardioCore_V1/schematics/CardioCore_V1.edif)).
> The connectivity below is **extracted from that netlist** (factual). All component
> **values** are **candidates that MUST be verified** against the ADS1298 datasheet
> (TI SBAS459), the ADS1298ECG-FE reference design, and bench testing before PCB
> layout. Do not treat any value here as final.

> **Research / education use only.** Not a medical device; no diagnosis, treatment,
> monitoring, emergency, or clinical use. See [Safety_Research_Use.md](./Safety_Research_Use.md).

---

## 1. Method

The `CardioCore_V1.edif` netlist was parsed to recover every net and its endpoints,
then the ADS1298 (`U2`, `ADS1298IPAGR`) pin → net connectivity and the per-channel
input network were reconstructed. 61 nets and 64 components were resolved.

---

## 2. Supply & reference configuration (extracted)

| Item | Finding |
|---|---|
| Analog supply | `AVDD`, `AVDD1` → **3V3**; `AVSS` → **GND** ⇒ single **~3.3 V unipolar** analog supply |
| Digital supply | `DVDD` → **3V3**; `DGND` → **GND** (shared 3.3 V rail from AP2112K) |
| Reference | `VREFP` → **VREF_2V5** (REF5025 `U5` V_out, decoupled by `C9`) |
| VCAP | `VCAP1..4` → `C10..C13` |
| RLD ref | `RLDREF` → `C14` |
| Clock | `CLKSEL` → **GND** ⚠️ (selects **external clock**; see Finding F1) |

**Note (reference range):** with a ~3 V analog supply TI suggests VREFP ≈ AVSS + 2.4 V.
REF5025 = 2.5 V on a 3.3 V AVDD is acceptable (extra headroom), but confirm the internal
reference buffer is disabled (`CONFIG3`) and VREFP headroom/noise at bench.

---

## 3. Electrode connector (J3) pinout & channel mapping (extracted)

`J3` (Molex 5034801000, FFC/FPC) pin order, from the netlist:

| J3 pin | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|
| Electrode | RA | LA | LL | RL/RLD | V1 | V2 | V3 | V4 | V5 | V6 |

**ADS1298 channel assignment (as wired):**

| Channel | IN_P | IN_N | Series R (P / N) | CM cap (P / N) | Diff cap | Notes |
|---|---|---|---|---|---|---|
| 1 | RA | LA | R9 / R10 | C15 / C16 | C25 | ≈ −Lead I (RA−LA) |
| 2 | LL | V1 | R11 / R12 | C17 / C18 | C26 | |
| 3 | V2 | V3 | R13 / R14 | C19 / C20 | C27 | |
| 4 | V4 | V5 | R15 / R16 | C21 / C22 | C28 | |
| 5 | V6 | AUX | R17 / R18 | C23 / C24 | C29 | AUX has **no electrode** (see F5) |
| 6–8 | — | — | — | — | — | routed to expansion header **J5** (pins 15–20) |

Signal path per line: `electrode → J3 → ESD array (D1/D2) → series R (R9–R18) → [CM cap to GND, diff cap across pair] → ADS1298 INx`.

> **⚠️ Montage observation (Finding F2):** the inputs are wired as **5 differential
> electrode pairs**, not as precordial leads referenced to a Wilson Central Terminal
> (WCT). As wired, you cannot directly read standard V1–V6 precordial leads; you get
> RA−LA, LL−V1, V2−V3, V4−V5, V6−AUX. Decide whether this differential montage is
> intended, or whether the precordials should be referenced to WCT (the ADS1298 can
> generate/route WCT).

---

## 4. Input network topology (extracted)

Per channel, a classic ECG anti-alias / EMI filter is in place:

```text
         R(series)            ADS1298
electrode ──/\/\── INxP ──┬───────── INxP
  (via ESD)               │
                        [Ccm]        (Ccm to GND on each leg)
                          │
                         GND
              ── across the pair ──[Cdiff]──   (between INxP and INxN)
```

- **Series resistors** `R9–R18` (10) — one per input line.
- **Common-mode caps** `C15–C24` (10) — one per line to GND.
- **Differential caps** `C25–C29` (5) — one across each INxP/INxN pair.
- All values currently **TBD** (`R9–R18`, `C15–C29` show as TBD in the BOM).

---

## 5. RLD / right-leg-drive network (extracted)

| Net | Endpoints | Meaning |
|---|---|---|
| `RLD_DRIVE_OUT` | `U2.RLDOUT`, `R19`, `R20`, `C30` | RLD amp output |
| `ECG_RL_RLD` | `R19`, `J3.4` | drive to the RL electrode via series R19 |
| `RLD_FB_TBD` | `U2.RLDIN`, `U2.RLDINV`, `R20`, `C30` | feedback network (R20 ∥ C30) |
| `RLDREF` | `U2.RLDREF`, `C14` | RLD reference decoupling |

So: `RLDOUT → R20 ∥ C30 → RLDIN/RLDINV` (feedback) and `RLDOUT → R19 → RL electrode`.
The feedback net is literally named **`RLD_FB_TBD`** — values are open. `RLDIN` and
`RLDINV` are tied together here (see F8).

---

## 6. SPI / control / power nets (extracted, sanity check)

- SPI bus `SPI_SCLK/MOSI/MISO` is **shared** by `U2` (ADS1298), `J2` (microSD), and `J5`
  (expansion); chip-selects are separate (`ADS_CS` for U2, `SD_CS` for microSD). ✔
- Control: `~CS`/`~DRDY`/`START`/`~RESET`/`~PWDN` → ESP32-S3 GPIO IO10/14/15/16/17. ✔
- Charger `MP2662` I²C (`I2C_SDA/SCL`) + `CHG_INT`, `BATT_NTC` (R8). ✔
- USB-C `CC1/CC2` pull-downs `R1/R2`; `D+/D-` → ESP32 native USB. ✔

---

## 7. Decoupling check

`C1–C14` are all listed as **100 nF** in the BOM, but several of these nodes need
**µF-range** capacitance (see Finding F3):

| Node | Cap | Needs |
|---|---|---|
| `VREFP` (VREF_2V5) | C9 | ~1 µF (+ bulk, per ECG-FE) — **not** 100 nF alone |
| `VCAP1` | C10 | ~1 µF (datasheet); some designs up to 10 µF — verify |
| `VCAP2/3/4` | C11/C12/C13 | per datasheet (mix of nF/µF) |
| `3V3` rail | C3–C7 | per-pin 100 nF **+** bulk (ESP32 RF transients) |
| `USB_5V`/`VSYS` | C1/C2 | bulk (µF) |

---

## 8. Findings & action items (prioritized)

| ID | Sev | Finding | Suggested action |
|---|---|---|---|
| **F1** | 🔴 Critical | `CLKSEL` is tied to **GND** = external-clock mode (1–2.2 MHz), but **no clock source / CLK-pin connection** exists in the netlist. The device will not clock as wired. | For a single board, use the **internal oscillator**: strap `CLKSEL` **high (3V3)**. Or add a clock source on `CLK`. Verify against datasheet §"Clock". |
| **F2** | 🟠 High | Inputs are wired as **5 differential electrode pairs**, not WCT-referenced precordials. Standard V1–V6 leads are not directly available. | Decide intended montage; if standard precordials are wanted, reference them to **WCT** (route via ADS1298). Update channel/lead-mapping (B8). |
| **F3** | 🟠 High | `VREFP` / `VCAP1` (and bulk rails) are 100 nF placeholders; they need **µF-range** caps. | Set C9 (VREFP) and C10 (VCAP1) to ~1 µF (verify), add bulk; differentiate C1–C14 from a single 100 nF value. |
| **F4** | 🟡 Med | `VREFN` connection was **not found** in the netlist (should tie to **AVSS**). | Confirm VREFN → AVSS in the schematic. |
| **F5** | 🟡 Med | Channel 5 `IN5N` = **AUX** has **no electrode** (only D2.K5 / R18); it is effectively unused/floating. | Decide: leave as documented spare (bias/terminate properly) or reassign. |
| **F6** | 🟡 Med | No explicit DC **bias path** to mid-supply on the inputs; common-mode is established only via RLD. With DC coupling, inputs must stay in the ADS1298 input CM range. | Confirm input biasing keeps INx within CM range at startup / lead-off; consider bias resistors if needed. |
| **F7** | 🟢 Low | SPI bus shared across ADS1298 + microSD + expansion. | Confirm bus loading / max SCLK at target sample rate; OK in principle (separate CS). |
| **F8** | 🟢 Low | `RLDIN` and `RLDINV` are tied to the same feedback net. | Confirm this matches the intended RLD amp configuration. |

---

## 9. Candidate component values (⚠️ verify before layout)

> Starting points with rationale. **Verify** against ADS1298 datasheet (SBAS459) and
> the ADS1298ECG-FE reference design; finalize with bench/EMI testing. **Do not** treat
> as final.

**Input EMI / anti-alias filter** (per channel):

| Part | Role | Candidate | Rationale |
|---|---|---|---|
| R9–R18 | Series protection / filter R | **10 kΩ, 1 %** (0402) | Limits fault current, forms RC; 1 % matching for CMRR; balance vs Johnson noise. |
| C15–C24 | Common-mode cap → GND | **470 pF, C0G/NP0** | Small CM cap; keep matched (mismatch → CMRR loss). |
| C25–C29 | Differential cap (across pair) | **4.7 nF, C0G/NP0** | `Cdiff ≈ 10× Ccm` so the differential corner is set by Cdiff and tolerant to Ccm mismatch. |

Resulting corners (with R = 10 kΩ): differential `f ≈ 1/(2π·2R·Cdiff) ≈ 1.7 kHz`,
common-mode `f ≈ 1/(2π·R·Ccm) ≈ 34 kHz` — passes the ECG band, attenuates RF. Adjust
to the chosen ECG bandwidth.

**RLD network:**

| Part | Role | Candidate | Rationale |
|---|---|---|---|
| R20 | RLD feedback resistor | **~1 MΩ** | Sets RLD loop gain. |
| C30 | RLD feedback cap (∥ R20) | **~1–2 nF** | Low-pass for **loop stability**; pole ≈ `1/(2π·R20·C30)`. **Simulate/verify stability.** |
| R19 | Series R, RLDOUT → RL electrode | **a few kΩ – tens of kΩ** | Current-limit on the body-side drive. |
| C14 | RLDREF decoupling | **~1 µF** | Quiet RLD reference (internal `(AVDD+AVSS)/2` or external). |

**Reference / decoupling:** C9 (VREFP) ~1 µF + bulk; C10 (VCAP1) ~1 µF (verify; some
designs 10 µF); per-supply-pin 100 nF retained; add bulk on 3V3 / USB_5V / VSYS.

---

## 10. Next actions

1. Resolve **F1 (clock)** — it blocks bring-up. Re-strap `CLKSEL` or add a clock.
2. Decide the **montage / lead mapping (F2)** — this drives whether the board is rewired.
3. Apply candidate values, then **verify each** against the datasheet + a quick SPICE/bench check.
4. File these as GitHub issues using `.github/ISSUE_TEMPLATE/hardware_review.md` (one per finding).
5. Update [`Blockers_Before_PCB_Layout.md`](./Blockers_Before_PCB_Layout.md) as items close.

---

## Sources

- TI **ADS1298** datasheet (SBAS459): <https://www.ti.com/lit/ds/symlink/ads1298.pdf>
- TI **ADS1298ECG-FE** ECG front-end demo kit guide: <https://www.farnell.com/datasheets/1734626.pdf>

*Connectivity is extracted from the committed EDIF netlist; all component values are candidate/to-verify and not medical or final.*
