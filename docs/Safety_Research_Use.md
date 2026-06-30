# CardioVest / CardioCore V1 — Safety and Research-Use Statement

> ## ⚠️ READ THIS BEFORE POWERING THE BOARD OR CONNECTING ANY ELECTRODE TO A PERSON
>
> This document defines the **mandatory** electrical-safety and research-conduct
> rules for the CardioVest platform and the CardioCore V1 acquisition board.
> If you do not understand or cannot follow these rules, **do not connect this
> hardware to any person.**

---

## 1. What this device is — and is not

CardioCore V1 is an **8-channel ECG signal-acquisition board** built for
**research, prototyping, and engineering education**. It exists to let engineers
and researchers capture raw biopotential waveforms for experimentation,
algorithm development, and learning about analog front-end design.

### 🚫 Hard limits — these are absolute

- **THIS IS NOT A MEDICAL DEVICE.** It has no regulatory approval of any kind.
  No FDA clearance, no CE marking, no ISO 13485 / IEC 60601 conformance is
  claimed or implied. It must never be represented as a medical product.
- **NOT FOR DIAGNOSIS.** This hardware and its software must never be used to
  detect, identify, screen for, or rule out any medical condition.
- **NOT FOR TREATMENT.** It must never be used to guide, deliver, adjust, or
  withhold any therapy or medical intervention.
- **NOT FOR EMERGENCY MONITORING.** It must never be relied upon to watch a
  person's vital signs, raise alarms, or detect a deteriorating condition. It
  may fail silently at any time.
- **NOT DEFIBRILLATOR-PROOF.** This board has **no** defibrillator-protection
  circuitry and is **not** rated for defibrillation discharge. Never use it in
  any setting where a defibrillator might be applied.
- **NOT FOR USE ON OR NEAR ANYONE WITH AN IMPLANTED MEDICAL DEVICE** — for
  example a **pacemaker** or **implantable cardioverter-defibrillator (ICD)**.
  Do not place electrodes on, and do not run body-connected experiments near,
  any person who has such a device.

> **All data collected with this platform is EXPERIMENTAL ONLY.** It must never
> be used for any clinical purpose, shared as if clinically meaningful, or used
> to inform any health decision for yourself or anyone else.

---

## 2. ⚡ Electrical safety rules for body-connected testing

These rules apply any time **an electrode is, or may become, connected to a
living person** (including the researcher).

### 2.1 Battery power ONLY

- **Any body-connected testing MUST be performed on battery power ONLY** (the
  1S LiPo). The board must be fully self-contained and floating with respect to
  mains earth during measurement.
- **Do NOT measure ECG while the board is connected to a non-isolated USB
  source** — for example a mains-powered computer, a wall charger, a powered USB
  hub, or a docking station. A non-isolated USB connection ties the patient
  electrodes toward mains earth and defeats the floating-supply protection.
- **Do NOT charge the battery while measuring ECG on a person.** Charging
  requires USB power, which is exactly the non-isolated path you must avoid.
  Charging and body-connected measurement are **mutually exclusive** activities.

> **Rule of thumb:** if a cable from this board can be traced back to a wall
> outlet, **no electrode may be on a person.** Unplug everything, run on
> battery, *then* connect electrodes.

### 2.2 Acceptable workflows

| Activity                                   | USB connected? | Electrodes on a person? |
|--------------------------------------------|----------------|-------------------------|
| Charging the battery                       | Yes            | **NO**                  |
| Firmware flashing / serial debug           | Yes            | **NO**                  |
| Bench testing with a signal generator      | Optional       | **NO** (no person)      |
| Body-connected ECG capture                 | **NO**         | Yes (battery only)      |

---

## 3. Why electrical isolation matters

This is a plain-language engineering explanation, **not** medical guidance.

When electrodes are attached to a person, that person becomes part of the
electrical circuit. Skin contact provides a low-impedance path **into the body**.
If any part of the measurement system is referenced to mains earth, a fault or
even normal leakage in a mains-powered device can drive a small current through
the electrodes and through the body. This unwanted current is called **leakage
current**, and limiting it is the central concern of patient-connected
electrical safety.

Running the board on an **isolated, floating battery supply** removes the
conductive path to mains earth. There is no earth-referenced loop for leakage
current to flow through, so the energy available at the electrodes is bounded by
the small, isolated battery system rather than by the mains supply. This is the
single most important reason every body-connected experiment with CardioCore V1
must be **battery-powered and disconnected from any earthed equipment.**

Certified medical equipment achieves this protection with rigorously tested
isolation barriers, leakage-current limits, and fault-tolerance requirements
that this research board **does not** provide. The battery-only rule is a
necessary precaution — it is **not** a substitute for medical-grade isolation,
and it does not make this device safe for any clinical or unattended use.

---

## 4. ✅ Responsible testing checklist

Complete **every** item before placing any electrode on a person. If any item
fails, **stop** and do not proceed.

### Before the experiment
- [ ] I confirm this is a **research / educational** experiment only, with no
      diagnostic, treatment, or monitoring purpose.
- [ ] The test subject (which may be me) has **no implanted medical device**
      (pacemaker, ICD, or similar) and is not in proximity to anyone who does.
- [ ] The test subject has given **informed, voluntary consent**, understands
      this is experimental hardware, and can stop at any time.
- [ ] If required by my institution, I have the appropriate **ethics / IRB
      approval and supervision** for human-subject experimentation.

### Power and connections
- [ ] **All USB cables are unplugged** from the board.
- [ ] The board is running on its **1S LiPo battery only**.
- [ ] **Battery charging is OFF** and no charger is connected.
- [ ] No part of the board is connected to any **mains-earthed equipment**
      (computer, hub, charger, scope, dock).
- [ ] Battery condition looks normal: no swelling, damage, or excessive heat.

### Hardware integrity
- [ ] Electrode leads, connectors, and the board are **visually intact** — no
      frayed wires, exposed conductors, or damaged insulation.
- [ ] The right-leg drive (RL/RLD) and reference connections are wired as
      intended for this experiment.
- [ ] Firmware and configuration were verified **on the bench (USB)** *before*
      switching to battery-only, body-connected operation.

### During and after
- [ ] I am present and supervising for the entire session — this device is
      **never** left monitoring unattended.
- [ ] At the first sign of discomfort, heating, odor, or abnormal behavior,
      **disconnect the electrodes immediately.**
- [ ] When finished, **remove all electrodes from the person before**
      reconnecting USB or starting battery charging.
- [ ] Captured data is labeled and stored as **experimental research data only.**

---

## 5. Final reminder

CardioCore V1 is a tool for **building, measuring, and learning** — not for
caring for people. Treat every body-connected experiment as an electrical-safety
exercise first. When in doubt, **stay on battery, stay disconnected from mains,
and stop the experiment.**

> **This platform must never be used for diagnosis, treatment, emergency
> monitoring, or any clinical purpose. All results are experimental only.**
