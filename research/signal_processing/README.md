# CardioCore V1 — Signal-Processing Research

This directory holds exploratory signal-processing work for the **CardioVest /
CardioCore V1** research platform: experiments and algorithm prototypes that
operate on ECG-like waveforms captured by the 8-channel ADS1298 front-end.

> **Scope and disclaimer.** Everything here is for **research, prototyping, and
> education only**. The algorithms in this directory are *exploratory* and
> operate on *experimental* data. They are **not** diagnostic tools, are **not**
> a medical device, and must **never** be used for diagnosis, treatment, patient
> monitoring, emergency use, or any clinical decision-making. "R-peak detection"
> and "heart-rate estimation" here are studied purely as digital-signal-processing
> exercises, not as clinical interpretation of an ECG.

---

## Goals

The purpose of this research track is to characterize the raw signal produced by
the CardioCore V1 acquisition chain and to prototype common ECG-adjacent DSP
building blocks so we can evaluate hardware design choices (sampling rate, gain,
reference noise, electrode configuration, right-leg drive behavior, etc.).

Concretely, we want to answer engineering questions such as:

- How clean is the raw acquisition path at the default sample rate?
- Which filter designs best suppress baseline wander and powerline interference
  without distorting the waveform morphology?
- How robustly can a *simple* QRS/R-peak detector locate complexes in our
  experimental recordings, and where does it fail?
- What is the noise floor and noise character per channel?

These are evaluation questions about *our hardware and our algorithms* — not
about any subject's health.

---

## Planned experiments

### 1. Filtering

- **Baseline-wander removal.** Compare high-pass approaches (e.g. linear-phase
  FIR high-pass, low-cutoff IIR, and median/moving-average detrending) for
  removing slow drift from respiration and electrode motion. Evaluate phase
  distortion and impact on the ST segment / low-frequency morphology.
  - Candidate high-pass cutoff: **TBD** (to be chosen empirically from recorded
    drift content).
- **Powerline notch.** Design and compare notch / band-stop filters to attenuate
  mains interference. The platform is intended to support both **50 Hz** and
  **60 Hz** regions, so notch design should be region-selectable. Explore
  adaptive notch vs. fixed IIR notch vs. comb filtering of harmonics.
- **Anti-alias / band-limiting review.** Confirm the digital filter chain is
  consistent with the configured ADS1298 sample rate and on-chip filtering.
  - Final sample rate for experiments: **TBD**.

### 2. R-peak / QRS detection (algorithm exploration)

- Prototype classic detector(s) (e.g. a Pan–Tompkins-style pipeline:
  band-pass → derivative → squaring → moving-window integration → adaptive
  thresholding) **as a DSP study**, not as a clinical detector.
- Measure detector behavior against manually annotated experimental recordings:
  detection latency, false positives/negatives under added noise, and
  sensitivity to filter choices above.
- Explicitly document failure modes (motion artifact, lead-off, low SNR
  channels). These experiments characterize algorithm robustness only.

### 3. Heart-rate estimation (research exercise)

- Derive instantaneous and averaged rate estimates from detected R-R intervals
  produced by the detector above, treated strictly as a numerical exercise on
  experimental waveforms.
- Compare windowing/smoothing strategies and quantify estimate variance under
  different noise conditions.
- **Not** a heart-rate monitor and **not** for any health purpose.

### 4. Noise characterization

- Per-channel noise floor measurement with electrodes shorted and with a known
  test input.
- Power-spectral-density analysis to identify mains harmonics, reference
  (REF5025) noise contributions, and digital/switching artifacts.
- Input-referred noise vs. ADS1298 gain (PGA) setting.
- Effect of right-leg drive (RLD) on common-mode rejection in the experimental
  setup.

---

## How to organize work here

Keep experiments reproducible and self-documenting:

- Prefer **notebooks** (e.g. Jupyter) for exploratory analysis and figures, and
  **scripts** for reusable, parameterized processing.
- One experiment per file/notebook, with a clear, descriptive name.
- At the top of each notebook/script, document: **method**, **input data**
  (which recording, sample rate, gain, electrode config), **parameters**, and
  **what question it answers**.
- Record results (plots, metrics) alongside the code, and note the commit/hardware
  revision used.
- Treat all recordings as **experimental data**, never as health information.

Suggested layout:

```
signal_processing/
├── README.md            # this file
├── notebooks/           # exploratory analysis (filtering, detection, noise)
├── scripts/             # reusable processing / batch experiments
└── data/                # experimental recordings (or pointers to them)  [TBD]
```

---

## Reminder

These are exploratory algorithms applied to experimental data on an uncertified
research platform. Nothing here interprets an ECG clinically, and no output of
this work should be relied upon for any medical or safety purpose.
