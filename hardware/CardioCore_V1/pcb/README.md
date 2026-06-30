# CardioCore V1 — PCB Layout

> **Research / prototyping / education only.** CardioCore V1 is **not** a certified medical
> device and must not be used for diagnosis, treatment, patient monitoring, emergency use, or
> any clinical decision-making.

This folder will hold the physical board design for CardioCore V1:

- **PCB layout files** (designed in **flux.ai**; the live layout is in the project source [`../flux_project/CardioCore_V1.flx`](../flux_project/CardioCore_V1.flx))
- **Board renders** (top/bottom 3D exports, assembly views)
- **Stackup notes** (layer count, copper weights, dielectric, impedance targets — all **TBD**)

## Status: NOT STARTED

**PCB layout must not begin until the following are finalized:**

1. The CardioCore V1 schematic is complete and reviewed.
2. The ADS1298 analog front-end values are locked (input filtering / anti-alias network,
   right-leg drive network, REF5025 reference filtering and decoupling, electrode protection).

Starting layout before the analog design is settled risks expensive re-spins and degraded
signal quality on the high-impedance electrode inputs. Treat this folder as reserved until
those gates are cleared.

## High-level layout requirements

These are guiding constraints to keep in mind once layout begins. Specific numeric targets
(trace widths, clearances, stackup) are **TBD** and depend on the finalized schematic.

- **Analog / digital separation.** Keep the ADS1298 analog front-end, electrode inputs, and
  the REF5025 reference physically separated from the ESP32-S3, switching regulators, USB-C,
  BLE radio, and the microSD interface. Avoid routing noisy digital or switching traces under
  or near sensitive analog sections.
- **Quiet, stable reference.** Give the REF5025 2.5 V reference a clean local supply, careful
  decoupling, and a low-noise return path. The reference and the ADS1298 supplies set the noise
  floor for the whole acquisition chain — protect them.
- **Short, symmetric, high-impedance electrode traces.** Route the 10 electrode inputs
  (RA, LA, LL, RL/RLD, V1–V6) as short and direct as possible from connector to AFE. Keep
  paired/related traces matched, guard sensitive nets where appropriate, and minimize exposure
  to coupling from digital and power sections.
- **Proper grounding.** Use a deliberate ground strategy (continuous reference plane, controlled
  partitioning between analog and digital returns, single well-defined connection point).
  Avoid split-plane gaps under high-speed or sensitive signals that would force return-current
  detours.
- **Power integrity.** Local decoupling at the ADS1298, REF5025, and ESP32-S3; keep charging /
  regulation switching loops tight and away from the analog domain.

## Related documentation

- Architecture and signal-chain overview:
  [`../../../docs/CardioCore_Architecture_v1.md`](../../../docs/CardioCore_Architecture_v1.md)
- Pre-layout gating items (must be cleared first):
  [`../../../docs/Blockers_Before_PCB_Layout.md`](../../../docs/Blockers_Before_PCB_Layout.md)
