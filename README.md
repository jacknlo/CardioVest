# CardioVest

CardioVest is a wearable, multi-channel ECG (electrocardiography) acquisition **research platform**. It is being developed to explore high-fidelity, simultaneous-sampling biosignal capture in a small, battery-powered, wireless form factor suitable for benchtop experimentation, signal-processing research, and engineering education.

The first hardware module in the platform is **CardioCore V1** — an 8-channel, battery-powered ECG acquisition board built around the ESP32-S3 and the Texas Instruments ADS1298 analog front-end. CardioCore V1 handles electrode signal conditioning, simultaneous 8-channel sampling, on-device logging, and wireless streaming, and serves as the data-acquisition foundation for the broader CardioVest wearable concept.

---

> ## RESEARCH USE ONLY
>
> **CardioVest and CardioCore V1 are research, prototyping, and educational tools. They are NOT a medical device.**
>
> **This project is NOT intended, designed, or validated for diagnosis, treatment, patient monitoring, emergency use, or any clinical decision-making.** It must not be used to make health decisions for yourself or anyone else.
>
> No diagnostic or interpretive ECG algorithms are provided. The hardware and firmware have **no regulatory clearance or certification of any kind** (no FDA, CE, ISO, or equivalent status is claimed or implied). If you connect any hardware to a living subject, you are solely responsible for ensuring electrical safety and compliance with all applicable rules, including ethics/IRB approval where required.
>
> See [docs/Safety_Research_Use.md](docs/Safety_Research_Use.md) before building or operating any hardware.

---

## Project Status

- **Current status:** Repository organized as an engineering workspace. The schematic concept is captured in flux.ai (project source + EDIF netlist committed), but the **ADS1298 analog front-end values are still TBD**.
- **Next milestone:** **ADS1298 analog front-end verification** — finalize the input RC filter, series protection resistors, RLD/DRL network, and reference/decoupling against the datasheet.
- ⛔ **PCB layout is BLOCKED** until every item in [docs/Blockers_Before_PCB_Layout.md](docs/Blockers_Before_PCB_Layout.md) is resolved and the analog front-end is verified. No PCB layout or fabrication outputs (Gerbers) are produced yet.

**Project controls:** [TODO.md](TODO.md) · [docs/AFE_Verification_Report.md](docs/AFE_Verification_Report.md) · [docs/Blockers_Before_PCB_Layout.md](docs/Blockers_Before_PCB_Layout.md) · [docs/Design_Decisions_Log.md](docs/Design_Decisions_Log.md) · [docs/Claude_Code_Workflow.md](docs/Claude_Code_Workflow.md) · [hardware/CardioCore_V1/Project_Specification.md](hardware/CardioCore_V1/Project_Specification.md)

---

## Core Hardware (CardioCore V1)

CardioCore V1 is an 8-channel ECG acquisition board with the following key components:

- **MCU module:** Espressif **ESP32-S3-WROOM-1-N16R8** (16 MB flash / 8 MB PSRAM), providing Wi-Fi/BLE connectivity and compute headroom for buffering and on-device processing.
- **Analog front-end (AFE):** Texas Instruments **ADS1298** — 8-channel, 24-bit, simultaneous-sampling delta-sigma ECG front-end with integrated right-leg drive (RLD) and lead-off detection.
- **Voltage reference:** TI **REF5025** external **2.5 V** precision reference for the ADS1298.
- **Power:** Single-cell **1S LiPo** battery with onboard charging.
- **USB-C:** Used for battery charging and firmware programming/debugging.
- **Wireless:** **BLE** streaming of acquired channels.
- **Storage:** **microSD** card for on-device data logging.

### Electrode Inputs (10 total)

CardioCore V1 exposes 10 electrode connections supporting a standard limb + precordial electrode set:

- **Limb electrodes:** `RA` (right arm), `LA` (left arm), `LL` (left leg)
- **Reference / drive:** `RL/RLD` (right leg — right-leg drive)
- **Precordial electrodes:** `V1`, `V2`, `V3`, `V4`, `V5`, `V6`

> Note: electrode-to-lead derivation (e.g., how raw electrode channels are combined) is handled in firmware/software and is out of scope for the analog board itself. Specific channel mapping and lead derivation conventions are **TBD** and will be documented in the firmware notes.

## Planned Features

- **8-channel simultaneous acquisition** via the ADS1298 (24-bit, simultaneous sampling across channels).
- **BLE streaming** of live channel data to a host.
- **microSD logging** for untethered, on-device data capture.
- **USB-C charge & program** for a single-connector workflow.
- **Desktop viewer** for live visualization and recording of streamed/logged data.
- **Mobile app** companion for configuration and live viewing.
- **Signal-processing research** support — datasets, filtering/denoising experiments, and analysis tooling (no diagnostic interpretation).

## Repository Structure

```text
CardioVest/
├── README.md
├── LICENSE
├── .gitignore
├── TODO.md
├── .github/
│   └── ISSUE_TEMPLATE/
│       ├── hardware_review.md
│       ├── firmware_task.md
│       └── documentation_task.md
├── docs/
│   ├── CardioCore_Architecture_v1.md
│   ├── Safety_Research_Use.md
│   ├── ADS1298_Analog_Frontend_Notes.md
│   ├── Bringup_Plan.md
│   ├── Roadmap.md
│   ├── Glossary.md
│   ├── Blockers_Before_PCB_Layout.md
│   ├── Design_Decisions_Log.md
│   └── Claude_Code_Workflow.md
├── hardware/
│   └── CardioCore_V1/
│       ├── README.md
│       ├── Project_Specification.md
│       ├── flux_project/
│       │   └── CardioCore_V1.flx          # editable flux.ai source (schematic + PCB)
│       ├── schematics/
│       │   ├── README.md
│       │   └── CardioCore_V1.edif         # EDIF netlist export
│       ├── pcb/
│       │   └── README.md
│       ├── bom/
│       │   ├── preliminary_bom.md
│       │   └── flux_export/               # per-vendor BOM CSVs + original zip
│       └── manufacturing/
│           └── README.md
├── firmware/
│   └── esp32_s3/
│       ├── README.md
│       ├── src/
│       │   └── main.cpp
│       └── notes/
│           └── firmware_plan.md
├── software/
│   ├── desktop_viewer/
│   │   ├── README.md
│   │   └── viewer_plan.md
│   └── mobile_app/
│       ├── README.md
│       └── app_plan.md
└── research/
    ├── README.md
    ├── signal_processing/
    │   └── README.md
    └── datasets/
        └── README.md
```

- **docs/** — project-level documentation: see [Roadmap.md](docs/Roadmap.md) and [Safety_Research_Use.md](docs/Safety_Research_Use.md).
- **hardware/CardioCore_V1/** — schematics, PCB layout, bill of materials, and manufacturing/fabrication files for the CardioCore V1 board.
- **firmware/esp32_s3/** — ESP32-S3 firmware (`src/`) and engineering notes (`notes/`).
- **software/** — host-side tooling: a `desktop_viewer/` and a `mobile_app/`.
- **research/** — `signal_processing/` experiments and `datasets/` used for analysis.

## Development Roadmap

High-level order of work (see [docs/Roadmap.md](docs/Roadmap.md) for detail and milestones):

1. **Hardware design** — schematic capture and PCB layout for CardioCore V1 (ADS1298 + ESP32-S3 + REF5025 + power).
2. **Fabrication & bring-up** — manufacture the board, assemble, and validate power, references, and AFE communication.
3. **Firmware foundation** — ESP32-S3 drivers for the ADS1298, simultaneous sampling, and data buffering.
4. **Data paths** — microSD logging and BLE streaming of acquired channels.
5. **Host software** — desktop viewer for live/recorded data, followed by the mobile companion app.
6. **Signal-processing research** — datasets, filtering/denoising, and analysis experiments.

## Safety Note

Working with biopotential acquisition hardware involves electrical-safety considerations, especially if electrodes are ever connected to a person. CardioCore V1 is a research prototype with **no medical certification**, and improper connection (for example, mains-powered equipment in the signal path) can be dangerous. Read [docs/Safety_Research_Use.md](docs/Safety_Research_Use.md) in full before powering or connecting any hardware, and ensure you follow all applicable electrical-safety and research-ethics requirements for your context.

## License

The license for this project is **TBD**. See [LICENSE](LICENSE) for the current status.
