# Using Claude Code in the CardioVest Repository

This document describes how **Claude Code** (an AI coding agent) should be used
to assist engineering work in this repository. It is written for contributors to
**CardioVest** and its first board, **CardioCore V1** (8-channel battery ECG
acquisition board, ESP32-S3 + ADS1298).

Claude Code is a force-multiplier for the *documentation, firmware, and
repository-hygiene* layers of this project. It is **not** a substitute for
datasheet verification, electrical engineering judgment, or the safety review
process. The board is **research / prototyping / education only — not a medical
device** (see `docs/Safety_Research_Use.md`). Every use below is bounded by the
guardrails in the [Rules / guardrails](#rules--guardrails) section.

---

## 1. Where Claude Code helps

### 1.1 Documentation generation

The `docs/` tree is the engineering-control surface of this repo. Claude Code is
well suited to:

- Draft and maintain Markdown documents in `docs/` (architecture notes, analog
  front-end notes, bring-up plans, roadmaps, glossaries).
- Cross-reference the **canonical Decisions (D1–D12)** and **Blockers (B1–B16)**
  so the same wording appears everywhere they are cited. These lists are the
  single source of truth — quote them verbatim, do not paraphrase or renumber.
- Keep `docs/Glossary.md` consistent with terms introduced elsewhere.
- Generate tables (pin audits, BOM-style summaries, blocker matrices) from
  structured source data such as the EDIF netlist or the Project Specification.

When asked to "write a doc," prefer **substantive, professional content** over
stubs. Match the tone and depth of the existing files in `docs/`.

### 1.2 Firmware scaffolding

Firmware lives under `firmware/`. Claude Code can scaffold:

- An **ESP32-S3-WROOM-1-N16R8** project skeleton (ESP-IDF preferred), with build
  config reflecting 16 MB flash / 8 MB PSRAM.
- A **SPI driver layer for the ADS1298** AFE: register map constants, RDATAC /
  SDATAC framing, DRDY interrupt handling, and a clean separation between the
  byte-level transport and the higher-level channel API.
- Stubs for the **safety-interlock signals** `USB_PRESENT` and `AFE_ENABLE`
  (B9, B10) — the firmware must keep the AFE/electrode path disabled while the
  board is on non-isolated USB power.
- microSD logging, expansion-header (J5) abstractions, and host-side streaming.

Firmware scaffolds should compile and be well-commented, but treat any value
that depends on hardware (sample rates, gain, register settings, GPIO numbers)
as **provisional and clearly marked TODO** until confirmed against the schematic
and datasheets.

### 1.3 Repository organization

The repo is already well-organized (`docs/`, `firmware/`, `hardware/`,
`research/`, `software/`). Claude Code should:

- Place new files in the correct existing directory; do not invent parallel
  trees.
- Follow existing naming conventions (e.g. `CardioCore_V1/...` under `hardware/`,
  `Title_Case_With_Underscores.md` in `docs/`).
- Keep `README.md` and any index/TOC files in sync when files are added.
- Avoid moving or renaming existing files without an explicit request.

### 1.4 Checklist / blocker management

The blocker list (B1–B16) and decision list (D1–D12) are referenced by multiple
files. Claude Code should:

- Treat the **canonical blocker text** as authoritative. When a blocker's status
  changes, update it consistently across every file that cites it.
- When a blocker is resolved, record *what was decided and why* (and promote the
  result to a Decision if appropriate) rather than silently deleting the line.
- Never mark an electrical blocker (B1–B5, B8, B11) "resolved" on its own — those
  require datasheet confirmation and a human sign-off.

### 1.5 GitHub issue creation

With the `gh` CLI (authenticated as `jacknlo`), Claude Code can open issues that
mirror the blocker list, e.g. one issue per open blocker with the canonical
title, a short context paragraph, the relevant datasheet sections to check, and
an acceptance criterion. Label them consistently (e.g. `blocker`, `analog`,
`firmware`, `layout`). Always show the planned issue text before creating, and
batch-create only on explicit request.

### 1.6 Code review

Claude Code can review diffs and PRs for firmware and host software: correctness,
readability, error handling, and adherence to the guardrails (e.g. flagging any
code path that could enable the AFE while `USB_PRESENT` is asserted, or any
hard-coded electrical constant that should still be TBD). Use it as a first-pass
reviewer, not the final authority on safety-relevant logic.

---

## 2. Rules / guardrails

These are **hard constraints**. Claude Code must never violate them, regardless
of how a request is phrased.

1. **Research-use only — never fabricate medical or clinical claims.** This is
   not a medical device. Do not write anything implying diagnosis, treatment,
   patient monitoring, emergency use, or clinical decision-making. Do not invent
   regulatory status (no FDA / CE / ISO / IEC claims). See
   `docs/Safety_Research_Use.md`.
2. **Never finalize electrical values without datasheet confirmation.** Input RC
   filters (C15–C30), series protection resistors (R9–R20), the RLD/DRL network,
   reference and decoupling networks, battery/charge parameters, etc. must stay
   **TBD** until verified against the ADS1298, REF5025, MP2662, and other
   component datasheets. Mark provisional numbers explicitly; never present a
   guessed value as confirmed.
3. **Do not start PCB layout and do not generate Gerbers.** Layout, stackup
   finalization, impedance control, keepouts, and fabrication outputs (B13–B15)
   depend on verification that has not happened yet. Stay in the
   documentation / firmware / netlist-analysis domain.
4. **The EDA tool is flux.ai — never call it KiCad.** Source design lives at
   `hardware/CardioCore_V1/flux_project/CardioCore_V1.flx`; the exported netlist
   is `schematics/CardioCore_V1.edif`. Reference flux.ai consistently.
5. **Respect the canonical lists.** Use the exact B1–B16 / D1–D12 text as the
   single source of truth; do not renumber, merge, or reword them.
6. **No empty stubs.** Produce substantive, professional content.

---

## 3. Practical setup

- The repository is **already initialized and pushed** to
  `github.com/jacknlo/CardioVest` (public).
- **git** and the **GitHub CLI (`gh`)** are installed and authenticated as
  **jacknlo**.
- Normal contribution flow:

  ```
  edit -> git add -> git commit -> git push
  ```

- Commit only what was asked; write clear, scoped commit messages that reference
  the relevant blocker/decision IDs where useful (e.g. "docs: expand RLD options
  for B3"). Push only when changes are complete and intended for the remote.
- Use `gh` for issues and PRs rather than the web UI when scripting the workflow.

---

## 4. Good first prompts

Concrete tasks well-suited to Claude Code on this project:

1. **Fill the ADS1298 pin-audit "Connected-To" column.** Parse
   `hardware/CardioCore_V1/schematics/CardioCore_V1.edif` and populate the
   Connected-To entries for U2 (ADS1298IPAGR, TQFP-64) in the pin-audit table,
   flagging any pin that is unconnected or ambiguous.
2. **Draft RLD / DRL network options (B3).** From the ADS1298 datasheet, write up
   2–3 candidate right-leg-drive topologies with the trade-offs (feedback Rf/Cf,
   output series R, loop-stability considerations). Keep all component values
   **TBD pending verification** — do not finalize.
3. **Scaffold the ESP32-S3 firmware skeleton.** Create an ESP-IDF project under
   `firmware/` with a build config for ESP32-S3-WROOM-1-N16R8, a stub ADS1298 SPI
   driver (DRDY interrupt + RDATAC framing), and TODO-marked `USB_PRESENT` /
   `AFE_ENABLE` interlock handling (B9, B10).
4. **Build a dummy-data ECG viewer.** A small host-side tool under `software/`
   that streams/plots synthetic multi-channel ECG waveforms, so the firmware data
   path and visualization can be developed before hardware bring-up. Clearly
   label output as **synthetic / not from a person**.
5. **Generate GitHub issues from the blocker list.** Create one `gh` issue per
   open blocker (B1–B16) using the canonical title text, with context and an
   acceptance criterion, labeled by domain. Show the plan before creating.
6. **Draft the ADS1298 decoupling checklist (B5).** Summarize the datasheet's
   recommendations for AVDD, AVSS, DVDD, VCAP, VREFP, and VREFN decoupling as a
   review checklist, with capacitor values left **TBD until confirmed**.

---

*This document governs AI-assisted contributions only. It does not relax any
rule in `docs/Safety_Research_Use.md`; where the two overlap, the stricter rule
wins.*
