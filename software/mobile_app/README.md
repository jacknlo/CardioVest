# CardioVest Mobile App

Companion mobile application for the **CardioCore V1** acquisition board. It
connects over **Bluetooth Low Energy (BLE)** to visualize and record **raw,
uninterpreted ECG samples** for research, prototyping, and educational use.

> **Status: Planning.** No application code exists yet. This directory currently
> holds design notes only. See [`app_plan.md`](./app_plan.md) for the detailed
> plan.

---

## Intended Scope

- Scan for and connect to a CardioCore V1 device over BLE.
- Stream the raw 8-channel, 24-bit ECG sample frames produced by the board.
- Display live waveforms for selected channels/leads (research visualization).
- Record raw sample streams to local files for offline analysis.
- Show basic link/session metadata (device name, connection state, sample rate,
  dropped-frame counters).

The app handles **raw signal capture and display only**. It performs no signal
interpretation of any kind.

## Out of Scope

- No diagnosis, interpretation, or clinical reading of ECG signals.
- No alarms, alerts, patient monitoring, or emergency/clinical decision support.
- No claim of regulatory clearance or conformance.

## Tech Stack

**TBD.** The cross-platform framework is undecided — candidates under
consideration include **Flutter** or **React Native**. The choice will be
recorded in [`app_plan.md`](./app_plan.md) once made, along with the BLE
library and data/recording format decisions.

## Research-Use-Only Notice

CardioVest and CardioCore V1 are intended for **research, prototyping, and
education ONLY**. They are **NOT** certified medical devices and must **NOT** be
used for diagnosis, treatment, patient monitoring, emergency use, or any clinical
decision-making.

## See Also

- [`app_plan.md`](./app_plan.md) — detailed planning, architecture, and decisions.
