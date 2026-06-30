---
name: "Hardware review"
about: "Review a hardware design item (schematic, AFE, power, layout constraint) before it is committed"
title: "[HW] "
labels: ["hardware", "review"]
---

<!--
CardioVest / CardioCore V1 — research / prototyping / education only.
This is NOT a medical device. Do not make diagnostic, treatment, monitoring,
emergency, or clinical claims, and do not assert any regulatory status (FDA/CE/ISO).
EDA tool is flux.ai. Do not start PCB layout, generate Gerbers, or finalize
electrical values until the underlying datasheet checks are complete.
-->

## Area / block under review
<!-- e.g. ADS1298 input network, REF5025 reference, MP2662 power-path, RLD/DRL,
USB_PRESENT/AFE_ENABLE interlock, connector/lead mapping, layout constraint. -->



## Linked blocker(s)
<!-- Reference the canonical blocker IDs B1..B16 (single source of truth in the repo). -->

- 

## What was decided / proposed
<!-- Concise summary of the change or proposal. Reference canonical decisions D1..D12 if relevant. -->



## Datasheet / reference checked
<!-- Link the exact datasheet section, app note, or page used to justify each value/choice. -->

- [ ] Datasheet / reference linked:
- Relevant section(s)/page(s): 

## Risk / open questions
<!-- Signal integrity, leakage, safety interlock, thermal, mechanical, stability, etc.
List anything still uncertain or that depends on another blocker. -->

- 

## Acceptance checklist
- [ ] All values/choices are datasheet-verified (link provided above).
- [ ] No electrical values finalized prematurely — TBD items remain TBD until verified.
- [ ] Change is consistent with the canonical decisions (D1..D12) and blocker list (B1..B16).
- [ ] Scope is research / prototyping / education only — no medical-device behavior assumed.
- [ ] No medical, diagnostic, monitoring, emergency, or clinical claims.
- [ ] No fabricated or implied regulatory status (FDA/CE/ISO).
- [ ] EDA references use flux.ai (source `.flx`, netlist `.edif`); no PCB layout / Gerbers in this change.
- [ ] Body-connected safety concept respected (battery-only during measurement; USB charge/program only).
