# CardioVest Desktop Viewer

**Status: Planning** — no implementation yet. This README and the accompanying
[`viewer_plan.md`](./viewer_plan.md) define the intended scope and design space.

## Purpose

The Desktop Viewer is a research tool for **visualizing raw ECG samples** acquired
by the **CardioCore V1** acquisition board. It is intended to let an engineer or
researcher inspect signal quality, verify acquisition behavior, and review captured
data during development and experimentation.

Planned data sources:

- **Live BLE stream** — receive raw sample frames from CardioCore V1 over Bluetooth
  Low Energy and plot them in real time.
- **Imported logs** — load and replay recordings captured to **microSD** on the
  board, or **serial/USB** dumps from the host.

The viewer displays **raw, uninterpreted waveforms** across the available channels
(derived from the 10 electrode inputs: RA, LA, LL, RL/RLD, V1–V6). Intended features
are scoped in `viewer_plan.md` and include multi-channel time-series plots, channel
selection, basic pan/zoom, and export of selected segments for offline analysis.

## Tech Stack

**TBD.** The implementation language and framework are not yet decided. Candidates
under consideration:

- **Python + pyqtgraph** (fast real-time plotting, native desktop), or
- **Web-based** (browser UI, e.g. Web Bluetooth + a JS plotting library).

A final choice will be recorded in `viewer_plan.md` once evaluated.

## Scope and Non-Goals

This tool plots and reviews **raw signals only**. It does **not** perform ECG
interpretation, rhythm classification, or any form of automated analysis of clinical
significance. Such features are explicitly out of scope.

## Research Use Only

CardioVest and CardioCore V1 are for **research, prototyping, and education only**.
This is **not** a certified medical device and must **not** be used for diagnosis,
treatment, patient monitoring, emergency use, or any clinical decision-making. The
Desktop Viewer is a development and inspection aid, not a clinical instrument.

## See Also

- [`viewer_plan.md`](./viewer_plan.md) — detailed design notes, data format, and
  open questions for this tool.
