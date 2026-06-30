# CardioVest — Research

This directory is the home for **signal-processing experiments** and **dataset
handling** that support the CardioVest wearable multi-channel ECG acquisition
platform (first hardware module: **CardioCore V1**).

> **Scope and intent.** Everything in this directory is for **research,
> prototyping, and education only**. It is **not** a certified medical device,
> and **none** of the code, notebooks, methods, or results here are intended for
> diagnosis, treatment, patient monitoring, emergency use, or any clinical
> decision-making. Do not use any output from this work to make health
> decisions.

---

## Purpose

CardioCore V1 acquires raw, multi-channel ECG signals (up to 8 channels from a
10-electrode placement: RA, LA, LL, RL/RLD, V1–V6) using an ADS1298 analog
front-end. Raw acquisition is only half the story — to evaluate hardware design
choices, electrode placement, sampling configuration, and firmware behavior, we
need a place to:

- Explore and visualize captured signals.
- Develop and document **signal-processing** building blocks (filtering,
  baseline-wander removal, powerline-interference rejection, lead derivation,
  signal-quality metrics, etc.).
- Manage the **datasets** used as inputs to those experiments, with clear
  provenance and licensing.
- Keep experiments **reproducible** so that results can be re-run and reviewed.

This area is intentionally separate from firmware and hardware so that
exploratory analysis work does not get entangled with the embedded codebase.

> **Not in scope:** No diagnostic algorithms. No code that claims to interpret,
> read, or clinically classify ECG signals. Signal processing here is for
> engineering characterization and education — not for reading a heart trace.

---

## Subfolders

### `signal_processing/`
Experiments and reusable utilities for processing acquired ECG signals.
Typical content:

- Notebooks and scripts for filtering and conditioning (e.g., bandpass,
  notch/powerline rejection, baseline-wander correction).
- Signal-quality and characterization tools (noise floor, SNR estimates,
  channel comparison, lead-derivation math).
- Plotting/visualization helpers for inspecting captures.

These are **engineering** tools for understanding the hardware and signal chain.
They are explicitly **not** diagnostic or interpretive.

### `datasets/`
Storage, organization, and documentation for the data used in experiments.
Typical content:

- Pointers/metadata for **properly licensed open datasets**.
- The researchers' **own** experimental captures from CardioCore V1.
- A short data dictionary / provenance note per dataset (source, license,
  acquisition setup, date, and any preprocessing already applied).

See the **Ethics & Data** section below for hard rules on what may and may not
live here.

---

## Reproducibility

Research is only useful if it can be re-run and checked. Please follow these
practices:

- **Keep the notebooks and scripts** that produced a result in version control —
  not just the result. A figure or table without its generating code is not
  reproducible.
- **Document methods.** Each experiment should state, in prose or in the
  notebook header: what question it asks, the input data used (with its source),
  the processing steps and parameters, and the environment it was run in
  (language/library versions). Record acquisition settings where relevant
  (e.g., sample rate, gain, channel configuration) — mark anything not yet
  finalized as **TBD** rather than guessing.
- **Make randomness deterministic** where applicable (fix and record seeds).
- **Separate raw data from derived data.** Treat raw captures as immutable;
  write processed outputs to clearly named, separate locations.
- **Reference data by provenance, not by ad-hoc copies.** Note where a dataset
  came from and under what license so a reviewer can obtain it independently.

---

## Ethics & Data

These rules are **non-negotiable** for everything in this directory:

- **No real patient data.** Do not store, commit, or process clinical or
  identifiable patient recordings of any kind.
- **Allowed data only:** (1) the researchers' **own** experimental signals,
  captured with informed consent from the participant, and (2) **properly
  licensed open datasets** whose license permits the intended use. Always record
  the source and license.
- **Respect privacy.** Even for self-captured or open data, avoid storing
  personally identifying information alongside signals. Keep any participant
  details out of the repository.
- **Honor dataset licenses and attribution.** Follow the terms of any open
  dataset, including citation/attribution requirements and redistribution
  limits.
- **No clinical claims.** Do not present any analysis here as diagnostic,
  interpretive, or clinically validated.

---

## Reminder

Nothing in this directory is a medical device or a medical service. It produces
**no diagnosis** and is **not for clinical use**. It exists to support
engineering research, prototyping, and education for the CardioVest platform.
