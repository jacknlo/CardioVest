# CardioCore V1 — Schematics

This folder holds the schematic design sources, exported schematic PDFs, and
revision notes for the **CardioCore V1** board: an 8-channel, battery-powered
ECG acquisition board (ESP32-S3 + TI ADS1298 + REF5025).

> **Research / education only.** CardioCore V1 is a prototyping platform for
> research, engineering, and educational use. It is **not** a certified medical
> device and must not be used for diagnosis, treatment, patient monitoring,
> emergency use, or any clinical decision-making.

## Current status

**Not started.** No schematic source files exist yet. This README defines the
conventions to follow once design work begins.

## What goes here

- **Schematic sources** — the editable design files (see tool note below).
- **Exported PDFs** — a flattened, human-readable PDF export of each released
  schematic revision, committed alongside the source.
- **Revision notes** — a short changelog describing what changed between
  revisions and why (see `CHANGELOG` section below).

## EDA tool

- **Planned tool: KiCad — TBD.** KiCad is the intended schematic capture tool,
  but this is **not final**. The exact tool and version should be confirmed and
  recorded here before the first commit of design sources.

## Naming and versioning conventions

Use the following patterns (replace `?` with the integer revision number,
starting at `1`):

| Artifact            | Pattern                                   | Example                              |
|---------------------|-------------------------------------------|--------------------------------------|
| KiCad schematic     | `CardioCore_V1_schematic_rev?.kicad_sch`  | `CardioCore_V1_schematic_rev1.kicad_sch` |
| KiCad project       | `CardioCore_V1.kicad_pro`                 | `CardioCore_V1.kicad_pro`            |
| Exported PDF        | `CardioCore_V1_schematic_rev?.pdf`        | `CardioCore_V1_schematic_rev2.pdf`   |

Conventions:

- **Board name** (`CardioCore_V1`) stays fixed for this hardware module. A new
  major board revision (e.g. `V2`) lives in its own sibling folder, not here.
- **`rev?`** is the schematic revision and increments by `1` on each released
  change. Keep the source `.kicad_sch` and its matching PDF in sync (same `rev`).
- Use **underscores**, no spaces, and keep the casing exactly as shown.
- Record every revision in the changelog with date, author, and a one-line
  summary of the change.

## Changelog

Maintain revision history here (newest first). TBD until the first revision.

| Rev | Date       | Author | Summary        |
|-----|------------|--------|----------------|
| —   | TBD        | TBD    | Not started.   |

## Design consistency

Keep this schematic consistent with the project design documents:

- `docs/CardioCore_Architecture_v1.md` — overall board architecture, power tree,
  interfaces (USB-C charging/programming, BLE, microSD), and the 10 electrode
  inputs (RA, LA, LL, RL/RLD, V1–V6).
- `docs/ADS1298_Analog_Frontend_Notes.md` — ADS1298 analog front-end details,
  REF5025 2.5 V reference usage, and right-leg drive (RLD) considerations.

If a schematic decision diverges from these documents, update the documents (or
note the deviation here) so the design and docs stay aligned.
