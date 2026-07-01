# Contributing to CardioVest

Thanks for helping with **CardioVest** (and its first board, **CardioCore V1**). This is a
**research, prototyping, and education** project — please read the rules below before contributing.

## Hard rules (non-negotiable)

1. **Research / education only — not a medical device.** Never add or imply diagnosis, treatment,
   patient monitoring, emergency use, or clinical decision-making, and never claim regulatory status
   (FDA / CE / ISO). See [`docs/Safety_Research_Use.md`](docs/Safety_Research_Use.md).
2. **Don't finalize electrical values without a datasheet.** Input RC filters, series resistors, the
   RLD network, decoupling, charge parameters, etc. stay **TBD** until verified. Mark provisional
   numbers as candidates.
3. **No PCB layout / no Gerbers yet.** Layout is intentionally **blocked** — see
   [`docs/Blockers_Before_PCB_Layout.md`](docs/Blockers_Before_PCB_Layout.md).
4. **The EDA tool is flux.ai** (not KiCad). Source: `hardware/CardioCore_V1/flux_project/CardioCore_V1.flx`.

## Where things live (single sources of truth)

| Topic | Canonical file |
|---|---|
| Safety / scope | [`docs/Safety_Research_Use.md`](docs/Safety_Research_Use.md) |
| What blocks PCB layout | [`docs/Blockers_Before_PCB_Layout.md`](docs/Blockers_Before_PCB_Layout.md) (B1–B20) |
| Decisions made | [`docs/Design_Decisions_Log.md`](docs/Design_Decisions_Log.md) (D1–D12) |
| AFE verification | [`docs/AFE_Verification_Report.md`](docs/AFE_Verification_Report.md) |
| Next flux.ai edits | [`docs/Flux_Change_List.md`](docs/Flux_Change_List.md) |
| How AI agents are used | [`docs/Claude_Code_Workflow.md`](docs/Claude_Code_Workflow.md) |
| Near-term tasks | [`TODO.md`](TODO.md) |

## Workflow

- Build the firmware before pushing firmware changes: `cd firmware/esp32_s3 && pio run` (and
  `pio run -e cardiocore_v1_ads1292r` for the 2-channel bring-up target).
- Keep `README.md` (structure tree) and `docs/README.md` in sync when adding files.
- Reference blocker / decision IDs in commits where useful (e.g. `docs: expand RLD options for B3`).
- Open issues via the templates in `.github/ISSUE_TEMPLATE/`; PRs should satisfy the checklist in
  `.github/PULL_REQUEST_TEMPLATE.md`.

When the safety policy and any other doc conflict, **the safety policy wins.**
