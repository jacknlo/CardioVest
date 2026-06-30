# CardioVest — docs index

Documentation for the **CardioVest** research platform / **CardioCore V1** board.
**Research / education only — not a medical device.**

## Read first
- [Safety_Research_Use.md](Safety_Research_Use.md) — **the hard guardrail** (single source of truth for scope/safety).

## Architecture & hardware verification
- [CardioCore_Architecture_v1.md](CardioCore_Architecture_v1.md) — system architecture, blocks, power, expansion.
- [ADS1298_Analog_Frontend_Notes.md](ADS1298_Analog_Frontend_Notes.md) — AFE engineering notes (input network, RLD, reference, decoupling).
- [AFE_Verification_Report.md](AFE_Verification_Report.md) — **netlist-verified** connectivity, channel map, findings (F1–F8), candidate values.
- [Bringup_Plan.md](Bringup_Plan.md) — board bring-up stages 0–9.

## Project control (single sources of truth)
- [Blockers_Before_PCB_Layout.md](Blockers_Before_PCB_Layout.md) — **B1–B17**: everything that must be resolved before PCB layout.
- [Design_Decisions_Log.md](Design_Decisions_Log.md) — **D1–D12**: decisions made, with rationale.
- [Flux_Change_List.md](Flux_Change_List.md) — punch list to apply in flux.ai when access returns.
- [Roadmap.md](Roadmap.md) — milestones and future (Zio-inspired) ideas.

## Workflow & reference
- [Claude_Code_Workflow.md](Claude_Code_Workflow.md) — how AI agents are used in this repo (+ guardrails).
- [Glossary.md](Glossary.md) — terms used across the project.

See also the top-level [README.md](../README.md), [CONTRIBUTING.md](../CONTRIBUTING.md), and
the board specification at [hardware/CardioCore_V1/Project_Specification.md](../hardware/CardioCore_V1/Project_Specification.md).
