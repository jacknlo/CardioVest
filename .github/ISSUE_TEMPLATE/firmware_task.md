---
name: "Firmware task"
about: "Implement or modify ESP32-S3 firmware (acquisition, BLE, SD, control)"
title: "[FW] "
labels: ["firmware"]
---

<!--
CardioCore V1 — ESP32-S3-WROOM-1-N16R8 firmware.
RESEARCH / PROTOTYPING / EDUCATION ONLY. Not a medical device.
Firmware acquires, stores, and transmits RAW signal data only.
NO clinical interpretation, diagnosis, or decision-making logic.
-->

## Goal
<!-- One or two sentences: what should this firmware change accomplish? -->


## Affected module
<!-- Tick all that apply. -->
- [ ] ADS1298 SPI driver (config, RDATAC/RDATA, DRDY handling)
- [ ] Acquisition / sample pipeline (buffering, timestamps, framing)
- [ ] BLE GATT (services, characteristics, notifications)
- [ ] SD logging (FAT, file rotation, write throughput)
- [ ] Power / status (battery, charge state, USB_PRESENT, AFE_ENABLE interlock)
- [ ] Build / config / tooling
- [ ] Other: ___


## Description
<!--
Detail the change. Reference registers, pins, data formats, or
control flows as relevant. Note any expected behavior changes.
-->


## Dependencies / blockers
<!--
List dependent issues, PRs, or hardware items. Reference canonical
blockers (B1-B16) and decisions (D1-D12) where applicable.
Examples:
- B9 USB_PRESENT detection / B10 AFE_ENABLE interlock — required for
  safe acquisition gating in power/status firmware.
- B8 Lead / channel mapping — needed for correct INxP/INxN ordering.
-->


## Test plan
<!--
Describe how the change will be validated WITHOUT a human subject.
Prefer bench / synthetic methods:
- ADS1298 internal test signal (square wave) on selected channels.
- Dummy / replayed sample data through the acquisition pipeline.
- Loopback or shorted inputs; known DC levels via resistor divider.
- BLE: verify notification rate, payload integrity, reconnection.
- SD: verify file integrity, throughput, and no dropped frames.
-->


## Acceptance criteria
<!-- Concrete, checkable outcomes. -->
- [ ] Behavior matches the Description above.
- [ ] No regressions in existing acquisition / BLE / SD paths.
- [ ] Builds cleanly; configuration documented in-repo.
- [ ] Validated per the Test plan (bench / synthetic only).
- [ ] Firmware emits RAW data only — no interpretive output added.


> Note: This firmware handles RAW data only. It performs NO clinical
> interpretation, diagnosis, or patient-monitoring function. CardioVest
> is a research / prototyping platform and is not a medical device.
