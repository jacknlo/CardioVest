# CardioCore V1 — Desktop Viewer Plan

> Part of the **CardioVest** wearable multi-channel ECG acquisition research platform.
> Target hardware module: **CardioCore V1** (ESP32-S3 + TI ADS1298, 8-channel, 24-bit simultaneous-sampling AFE).

---

## ⚠️ Scope and Safety Notice

**This tool is for RESEARCH, PROTOTYPING, and EDUCATION ONLY. It is NOT a certified medical device.**

- The desktop viewer **visualizes raw acquired signal samples** for engineering and research purposes.
- It performs **NO diagnosis, NO interpretation, NO clinical reading, and NO patient monitoring** of any kind.
- It provides **NO diagnostic output**, no automated annotation of cardiac events, and no clinical decision support.
- Any signal processing offered (e.g., notch or high-pass toggles) is a **visualization aid only** — a cosmetic transform applied to the on-screen plot. It must never be described, used, or relied upon as a measurement, diagnostic, or clinical feature.
- The raw, unfiltered samples remain the source of truth for any downstream research analysis. Filters in this tool affect display only and must not silently alter exported raw data.

If any feature in this document risks implying clinical capability, the safety constraint above takes precedence and that feature must be reframed or removed.

---

## 1. Goals

The desktop viewer is the primary human-facing tool for inspecting data produced by CardioCore V1 during bring-up, characterization, and research data collection.

### Primary goals
1. **Real-time visualization** of raw multi-channel ECG samples streamed live from the device (BLE or serial), with low enough latency to be useful during bench work and electrode placement experiments.
2. **Offline visualization** of previously recorded sessions imported from microSD logs or saved stream captures.
3. **Faithful display of raw data** — what is plotted must be traceable back to the actual ADC counts / converted volts coming off the ADS1298, with no hidden processing.
4. **Researcher ergonomics** — quickly select channels, adjust scale/time-base, and zoom/pan to inspect artifacts, noise, saturation, lead-off events, and reference behavior.
5. **Export for downstream analysis** — emit clean CSV (and optionally other formats) so users can analyze data in Python/MATLAB/R, etc.

### Explicit non-goals
- No diagnosis, classification, or interpretation of cardiac signals.
- No "is this normal?" indicators, no arrhythmia/event detection, no heart-rate-as-clinical-metric reporting.
- No claim of accuracy, calibration, or conformance to any medical/regulatory standard.
- Not intended for use on, or decisions about, any person's health.

> A derived heart-rate or beat marker **could** appear in the future strictly as an engineering/debug overlay (e.g., to sanity-check sampling), but only if clearly labeled as a non-clinical research aid. This is **out of scope for the initial viewer** and listed only to record the boundary.

---

## 2. Data Ingestion

The viewer must accept the same logical sample stream regardless of transport. A transport-abstraction layer should normalize all sources into a common in-memory representation:

```
SampleFrame {
  timestamp        // device time and/or host-receive time (see format section)
  sequence_index   // monotonic counter for gap/drop detection
  channels[8]      // raw ADC counts (int32) + converted value (float, volts)
  status_flags     // lead-off / saturation / sync flags if provided by firmware (TBD)
}
```

### 2.1 BLE stream (live)
- Connect to CardioCore V1 over BLE GATT and subscribe to the streaming characteristic(s).
- Expected to carry batched, sequence-numbered frames (batching needed to fit ADS1298 throughput within BLE throughput limits).
- Must surface: connection state, RSSI (if available), throughput (frames/s, bytes/s), and **dropped/out-of-order frame count** via the sequence index.
- BLE service/characteristic UUIDs, payload packing, and batch size: **TBD** (must match firmware contract; see firmware docs once defined).

### 2.2 Serial (live)
- Connect over USB-C CDC serial for high-throughput, low-latency capture during bench work (typically faster and more reliable than BLE).
- Configurable port and baud/encoding; auto-detect candidate ports where possible.
- Same normalized frame model and same drop/gap accounting as BLE.
- Framing/encoding (e.g., COBS-delimited binary vs. line-based): **TBD** (must match firmware contract).

### 2.3 microSD log import (offline)
- Open a log file copied from the device's microSD card and replay it into the viewer.
- Support both playback (scrub/seek along the timeline) and full-file load for static inspection (subject to memory limits — large files may require windowed/streamed reads).
- Must validate file integrity where the format allows (e.g., header magic/version, sequence continuity) and clearly report gaps.

### 2.4 Stream capture import (offline)
- Allow saving a live BLE/serial session to disk and re-opening it later using the same on-disk format as microSD logs (one format, multiple producers) — see Section 4.

### Ingestion-wide requirements
- **No silent data modification.** Ingestion may reorder by sequence and flag drops, but must not interpolate/fabricate samples without an explicit, visible setting.
- Robust handling of partial frames, corrupted bytes, and disconnects (reconnect logic for live transports).
- Backpressure strategy for real-time: a bounded ring buffer drives the plot; if the UI cannot keep up, decimate **for display only** while preserving the full-rate stream for logging/export.

---

## 3. Features

### 3.1 Multi-channel time-series plotting
- Simultaneously plot up to all 8 acquired channels (mapped to the 10 electrode inputs RA, LA, LL, RL/RLD, V1–V6 per the firmware's derived-lead/channel mapping — mapping definition is **TBD** with firmware).
- Stacked lanes (one row per channel) and/or overlaid mode.
- Smooth, performant scrolling real-time render at the device sample rate (target render budget: keep up with full rate; decimate display only when necessary).
- Pan and zoom on both time and amplitude axes; jump-to-now and pause/freeze.

### 3.2 Channel selection
- Toggle channels on/off; reorder lanes; solo/mute a channel.
- Per-channel label, color, and visibility persistence within a session.
- Show per-channel live stats for debugging (min/max/peak-to-peak, current value, saturation indicator) — presented as **engineering diagnostics of the signal chain, not clinical metrics**.

### 3.3 Scaling and grid
- Per-channel and global amplitude scaling (auto-scale, fixed gain, and fit-to-window).
- Display units selectable between **raw ADC counts** and **converted volts** (using the configured ADS1298 gain and the REF5025 2.5 V reference; conversion constants sourced from device config — **TBD**).
- Configurable time-base (window length, e.g., seconds/division).
- Optional grid overlay with adjustable divisions. Any "mm/mV" or "mm/s" style grid, if offered, must be labeled as a **display convention for visualization only**, not a calibrated clinical chart.

### 3.4 Research-only display filters (visualization aids)
> **These are visualization aids only.** They transform the on-screen trace to make features easier to see during research. They are **not** measurement or diagnostic tools and **do not** alter the raw data or raw export.

- **Notch toggle** (e.g., 50/60 Hz mains rejection) — clearly labeled "Display filter — visualization aid only."
- **High-pass toggle** (baseline-wander removal for viewing) — same labeling.
- Optional low-pass / display smoothing — same labeling.
- A persistent on-screen badge indicates when **any** display filter is active, so a viewer never mistakes a filtered trace for raw data.
- A one-click "Show raw (filters off)" control.
- Filter parameters (cutoffs, order) are exposed and shown in any screenshot/export metadata so results are reproducible.

### 3.5 Export to CSV
- Export the current selection (channels × time window) or the full session.
- Default export is **raw data** (ADC counts and/or converted volts), independent of any active display filter.
- Include a metadata header/sidecar capturing: device/firmware identifiers (if available), sample rate, channel-to-lane mapping, gain/reference settings, timestamp base, and the exact filter state if a *filtered* export is explicitly requested.
- Filtered export, if offered, must be a separate, clearly-named option (e.g., `*_displayfiltered.csv`) and must record which filters were applied. Raw export must always remain available.
- Optional additional export targets (e.g., Parquet/NPY) are **TBD** based on user demand.

### 3.6 Session and quality indicators (debug)
- Live indicators for dropped/out-of-order frames, effective sample rate, buffer fill, and transport health.
- Lead-off / saturation status surfaced from firmware status flags where available (**engineering signal-integrity indicators, not clinical alerts**).
- Timeline markers/annotations the researcher can place manually (e.g., "electrode repositioned") — free-text, non-clinical.

---

## 4. Proposed File / Log Format (TBD)

A single on-disk format should serve both microSD logs and host-side stream captures so the import path is unified.

**Status: TBD.** Candidate directions (to be decided jointly with firmware):

| Option | Pros | Cons |
| --- | --- | --- |
| **Binary records + header** (magic, version, sample rate, channel map, gain/ref, then packed int32 samples + sequence + flags) | Compact; fast write on the embedded side; preserves full precision; cheap to stream | Needs a documented spec + parser on both sides; less human-inspectable |
| **CSV / line-based text** | Human-readable; trivial to import anywhere | Large; slower to write on MCU; precision/format ambiguity; weak for high sample rates |
| **Hybrid: binary payload + sidecar JSON/YAML metadata** | Clean separation of metadata vs. bulk samples; easy to extend metadata | Two files to keep together |
| **Existing biosignal containers (e.g., EDF/EDF+, WFDB, BDF)** | Interoperable with research tooling | May imply clinical framing; mapping our raw research stream may be awkward; added dependency/complexity |

**Minimum required metadata (whatever the container):** format magic + version, device/firmware ID (if available), UTC and/or device-clock base, sample rate, channel count and channel→electrode/lane mapping, ADS1298 gain, reference voltage (REF5025 2.5 V), and units convention. Sequence numbers and status flags should be preserved per frame.

**Decision criteria:** embedded write cost, precision fidelity, drop/gap recoverability, file size at target sample rate, and ease of parsing in Python/MATLAB. To be finalized once firmware sample rate and BLE/serial batch contract are fixed.

---

## 5. Candidate Tech Stack (TBD)

**Status: TBD.** The viewer is performance-sensitive (8 channels of 24-bit data, potentially high sample rate, real-time render). Candidate stacks and trade-offs:

| Stack | Pros | Cons | Notes |
| --- | --- | --- | --- |
| **Python + PyQtGraph (Qt)** | Fast scientific plotting tuned for real-time streams; huge ecosystem (NumPy/SciPy) for filters and CSV/Parquet; quick to prototype; cross-platform | Packaging/distribution heavier; GIL/perf care needed at high rates; native BLE/serial via libs (e.g., `bleak`, `pyserial`) | Strong default for a research tool; analysis code shares the Python ecosystem |
| **Python + Dear PyGui / matplotlib** | DPG is GPU-accelerated and snappy; matplotlib familiar for static/offline | matplotlib too slow for live high-rate; DPG smaller ecosystem | matplotlib good for offline/export figures, not live |
| **Web (TypeScript) + WebGL plotting (e.g., uPlot / regl)** | Excellent rendering perf; easy cross-platform UI; Web Bluetooth/Web Serial for live (browser support varies) | Web Bluetooth/Serial support and throughput limits vary by browser/OS; large-file handling trickier | Good if a browser-based or Electron app is desired |
| **Rust (egui/eframe) or C++ (Qt/ImGui)** | Top-tier performance and tight serial/BLE control; single binary distribution | Slower to iterate; smaller analysis/filter ecosystem than Python | Best if real-time performance becomes the binding constraint |

**Decision criteria:** real-time render performance at target sample rate, native BLE + serial support quality on Windows/macOS/Linux, ease of implementing display filters, ease of packaging/distribution to researchers, and reuse of the same language for downstream analysis. To be finalized after a short spike measuring sustained throughput on representative hardware.

---

## 6. Architecture Sketch (informational)

```
[ Transport layer ]      BLE  /  Serial  /  File replay
        |  (normalized SampleFrame stream)
        v
[ Ingest + ring buffer ] sequence/drop tracking, backpressure, raw retention
        |
        +--> [ Logger / capture writer ]  -> on-disk format (Sec. 4)
        |
        +--> [ Display pipeline ] decimation (display only) -> optional display filters (Sec. 3.4)
                    |
                    v
              [ Plot / UI ]  channels, scaling, grid, markers, quality indicators
                    |
                    v
              [ Exporter ]  raw CSV (default) / filtered CSV (explicit) / other (TBD)
```

Raw samples flow to the logger/exporter **unmodified**; display filters branch off only toward the screen.

---

## 7. Milestones

**M0 — Spec alignment**
- Lock the firmware data contract (sample rate, BLE/serial framing, batch size, channel→electrode mapping, status flags). Choose tech stack via a throughput spike. Draft the file/log format.

**M1 — Offline MVP**
- Import a microSD/capture file and render multi-channel time-series with pan/zoom and channel selection. Raw-CSV export. Establishes data model, parser, and plotting core.

**M2 — Live serial**
- Real-time serial ingestion with ring buffer, drop/gap accounting, effective-rate display, pause/freeze, and live capture-to-file (same format as M1).

**M3 — Live BLE**
- BLE transport behind the same abstraction; connection management, reconnect, throughput/RSSI indicators.

**M4 — Display filters + scaling/grid polish**
- Notch / high-pass / smoothing toggles as clearly-labeled visualization aids with the active-filter badge; per-channel and global scaling, units toggle (counts/volts), grid, and "show raw" control.

**M5 — Research UX & export polish**
- Manual timeline markers, per-channel debug stats, configurable layouts/persistence, export metadata sidecar, optional extra export formats (TBD).

**M6 — Hardening & docs**
- Performance tuning at target sample rate, large-file handling, robustness on disconnects/corruption, cross-platform packaging, and user documentation that restates the research-only / no-diagnostic-output scope.

---

## 8. Open Questions (TBD)

- Final device sample rate and ADS1298 configuration (gain, data rate) — drives buffer/perf budgets.
- BLE and serial wire formats and batch sizes (firmware contract).
- Channel→electrode/lane mapping and which derived leads, if any, firmware computes vs. the host.
- On-disk format choice (Section 4) and whether to interoperate with existing biosignal containers.
- Tech stack choice (Section 5) after the throughput spike.
- Conversion constants and where they live (device config vs. host config) for counts↔volts display.

---

*Reminder: CardioCore V1 and this viewer are research/prototyping/education tools only. They are not certified medical devices and produce no diagnostic output.*
