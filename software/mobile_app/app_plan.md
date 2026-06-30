# CardioVest Mobile App — Plan (CardioCore V1)

> **Scope and safety notice (read first).**
> This mobile application is part of the **CardioVest** research platform and pairs with the
> **CardioCore V1** acquisition board. It is intended **strictly for research, prototyping, and
> education**. It is **NOT a certified medical device**.
>
> The app **only displays and records raw research signals** streamed from CardioCore V1. It
> performs **NO diagnosis, NO interpretation, NO clinical reading, NO treatment guidance, and NO
> patient monitoring**. It must not be used for diagnosis, treatment, emergency use, or any
> clinical decision-making. No regulatory status (FDA / CE / ISO, etc.) is claimed or implied.

---

## 1. Purpose

The mobile app is the primary field-side companion for CardioCore V1. It lets a researcher:

- Discover and connect to a CardioCore V1 board over Bluetooth Low Energy (BLE).
- Visualize the live raw sample stream from the 8 ADS1298 channels in real time.
- Record timestamped research sessions to local device storage.
- Export and share those raw recordings for offline analysis in other tools.

It is deliberately a **thin acquisition + visualization client**. All signal-quality judgement,
analysis, and any downstream processing happen **outside** this app, by the researcher, using
general-purpose analysis tooling.

---

## 2. Goals

### 2.1 In scope (V1 targets)

1. **BLE connection to CardioCore V1**
   - Scan for advertising CardioCore V1 devices, filter by service UUID / name prefix.
   - Connect, discover the GATT profile, subscribe to the sample-stream characteristic.
   - Surface connection state clearly (scanning / connecting / streaming / disconnected / error).

2. **Live visualization of raw samples**
   - Plot raw, unprocessed samples per channel (counts or device-reported units) as scrolling
     waveforms.
   - Display the channel labels as wired (RA, LA, LL, RL/RLD, V1–V6 mapping is configured on the
     board; the app shows whatever channel order the device reports).
   - Show running stream stats: effective sample rate, packets/s, dropped-packet indicator.
   - **No filtering presented as clinically meaningful and no measurements** beyond optional,
     clearly-labeled *display-only* helpers (e.g. visual gain/offset for viewing). Display helpers
     never alter recorded raw data.

3. **Session recording**
   - One-tap start/stop of a recording session.
   - Persist the raw sample stream plus metadata (start time, device id, firmware version if
     advertised, declared sample rate, channel count/order, app version).
   - Recordings stored locally on the phone; survive app restart.

4. **Export / share of research recordings**
   - Export a recording (or batch) in a documented, analysis-friendly format (see §6, format TBD).
   - Share via the OS share sheet (files, email, cloud drive, USB transfer) and/or direct file
     export.

5. **Voice / event annotations (on-device)**
   - Let the researcher drop a **timestamped marker** on the ECG timeline — spoken (transcribed
     on-device), typed, or from the board button — to flag moments of interest (see §5.5).

### 2.2 Explicitly out of scope

- Any diagnosis, classification, anomaly flagging, arrhythmia detection, or interpretation.
- Any clinical/patient framing, alarms, or "is this normal?" features.
- Any claim of measurement accuracy suitable for clinical use.
- Closed-loop control of stimulation or therapy of any kind.

---

## 3. Architecture overview

```
+--------------------+        BLE GATT        +-------------------------+
|  CardioCore V1     | <--------------------> |   Mobile App            |
|  (ESP32-S3 + AFE)  |   notify: samples      |                         |
+--------------------+                        |  BLE layer              |
                                              |    - scan / connect     |
                                              |    - subscribe / parse  |
                                              |    - reconnect logic    |
                                              |                         |
                                              |  Stream pipeline        |
                                              |    - decode packets     |
                                              |    - ring buffer        |
                                              |    - stats (rate/drops) |
                                              |          |              |
                                              |    +-----+------+       |
                                              |    |            |       |
                                              |  Live plot    Recorder  |
                                              |  (render)     (storage) |
                                              |                  |      |
                                              |              Export/Share|
                                              +-------------------------+
```

Key principle: a **single decode + ring-buffer stage** feeds both the live plot and the recorder,
so what is shown and what is recorded come from the *same* raw bytes. The recorder writes raw data;
the plot may apply display-only transforms that never touch the recorded stream.

---

## 4. BLE concerns

> The board-side GATT contract is owned by the firmware. The values below are **TBD** until the
> CardioCore V1 firmware fixes them; this section captures requirements and open questions.

### 4.1 GATT service / characteristics (TBD)

| Element | Purpose | Status |
|---|---|---|
| Service UUID | CardioCore data service, used for scan filtering | **TBD** |
| Sample-stream characteristic | NOTIFY: packed multi-channel sample frames | **TBD** |
| Control characteristic | WRITE: start/stop stream, set sample rate, etc. | **TBD** |
| Status / info characteristic | READ/NOTIFY: fw version, channel count/order, declared rate, battery | **TBD** |
| Device info | Standard DIS (manufacturer, model, fw) if used | **TBD** |

**Open questions for firmware coordination:**
- Sample frame layout: channel count, per-sample width (ADS1298 is 24-bit), endianness, byte order.
- Packing: samples-per-notification, sequence/counter field for drop detection, timestamp source.
- Whether the board sends device counts (raw) only, or also declared scale factors (REF5025 2.5 V
  reference + ADS1298 PGA gain) so the app can label axes — app must still **record raw**.
- Control protocol for sample rate selection and stream start/stop.

### 4.2 Throughput

8 channels of 24-bit data are bandwidth-heavy; BLE is the bottleneck, not the AFE.

- Rough order-of-magnitude: 8 ch x 3 bytes x sample_rate. At, e.g., 500 SPS that is ~12 kB/s of
  payload before overhead — **sustained, continuous**, which is demanding for BLE notifications.
- Requirements / levers (target values **TBD**, to be measured):
  - Negotiate the **largest practical MTU** (ATT MTU / data length extension) to reduce per-packet
    overhead.
  - Negotiate a **short connection interval** for higher notify throughput.
  - **Batch multiple samples per notification** (firmware side) rather than one sample per packet.
  - Consider an optional **on-board compression / reduced sample rate** mode for constrained links
    (TBD; must remain lossless or clearly labeled if lossy — recordings must document this).
  - App must handle bursts without UI stalls: BLE callbacks feed a lock-free/bounded ring buffer;
    rendering and disk I/O run off the BLE thread.
- The app must **measure and surface** effective throughput, achieved sample rate, and dropped
  packets so the researcher knows the true fidelity of any recording.

### 4.3 Reconnection and robustness

- Auto-reconnect on unexpected disconnect, with capped exponential backoff.
- Distinguish *user-initiated* disconnect from *link loss* (do not auto-reconnect after a manual
  disconnect).
- On reconnect during an active recording: clearly mark a **gap** in the recording (with timestamps
  and a sequence discontinuity marker) rather than silently stitching data.
- Handle OS BLE quirks: permission revocation, Bluetooth turned off mid-session, app
  backgrounding/throttling, and platform-specific GATT caching.
- Watchdog: if no notifications arrive for a configurable timeout while "streaming", flag a stall
  and attempt recovery.
- Graceful behavior on low board battery / board-initiated shutdown (driven by status
  characteristic, TBD).

---

## 5. UI screens

### 5.1 Scan / Connect
- List of discovered CardioCore V1 devices (name, id, RSSI), filtered by service UUID / name prefix.
- Pull-to-refresh / continuous scan toggle; clear empty + permission-needed states.
- Tap to connect; progress and error feedback. Remember last device for quick reconnect.

### 5.2 Live plot
- Scrolling multi-channel waveform view (per-channel lanes or stacked), channel labels as reported.
- Controls: pause/resume view, time window (seconds/screen), per-channel display gain/offset
  (display-only), show/hide channels.
- Header readouts: connection state, effective sample rate, packets/s, dropped-count, board battery
  (if available).
- Prominent **Record** start/stop control with elapsed time and storage-used indicator.
- Persistent on-screen reminder that data is **raw research signal, not for diagnosis**.

### 5.3 Recordings list
- Chronological list of stored sessions: name/timestamp, duration, channel count, sample rate,
  file size.
- Per-item actions: rename, view metadata, **export / share**, delete (with confirm).
- Multi-select for batch export/delete. Storage usage summary.

### 5.4 Settings
- BLE: preferred MTU / connection-interval requests (where the OS allows), scan filter, auto-
  reconnect toggle and backoff.
- Recording: default sample rate request to board, storage location, file-naming scheme.
- Export: default format and options (see §6, TBD).
- Display: default time window, theme, units shown (counts vs declared units, clearly labeled).
- About: app + firmware versions, build info, and the full **research-only / no-diagnosis**
  disclaimer and license.

---

### 5.5 Voice & event annotations (symptom marking)

A spoken "symptom marker": the researcher taps **Annotate** (or presses a button on the board),
speaks a short note, and the app saves a **timestamped text annotation aligned to the ECG
timeline** — the voice version of the Zio-style event button (see `docs/Roadmap.md`).

**Design (chosen):**
- **On-device speech-to-text** (Android `SpeechRecognizer` / iOS `Speech`, via the framework's
  plugin if cross-platform). **No audio leaves the phone.**
- **Store text + timestamp only** — the raw audio is discarded right after transcription (lighter
  and more private). A "keep audio" option could be added later, off by default.
- **Time alignment ("exact moment"):** an annotation references the **ECG session timeline** —
  `t_ms` since session start (and `sample_index` once the firmware stamps frames), *not* phone
  wall-clock — so it lands on the right spot on the trace despite BLE latency. ~1 s accuracy is
  ample for symptom marking.
- Same track also accepts a typed note and the board-button press (one unified marker stream).

**Annotation track format** (`cardiovest.annotations.v1`, a JSON sidecar exported with the
recording — see §6):

```json
{
  "schema": "cardiovest.annotations.v1",
  "session_id": "2026-06-29T2210Z-cc1",
  "started_utc": "2026-06-29T22:10:04Z",
  "sample_rate_sps": 500,
  "annotations": [
    { "t_ms": 132450, "sample_index": 66225, "type": "voice",
      "text": "senti uma palpitacao agora", "source": "on-device-stt", "confidence": 0.86 },
    { "t_ms": 145000, "type": "button", "text": "" }
  ]
}
```

- `type`: `voice` | `button` | `note`. `sample_index` is optional (present once frames are stamped).
- The viewer overlays these as markers on the trace so analysts can jump to each event.

**Privacy:** voice + health context is sensitive. On-device only, explicit consent, raw audio not
retained, and the standard **research-only / no-diagnosis** framing. A transcript is a researcher's
note — **not** an interpretation of the ECG.

---

## 6. Data export format (TBD)

The recording/export format is **TBD**, but must satisfy these requirements:

- **Raw-first:** stores device-reported raw samples losslessly. Any display transforms are *not*
  baked into the exported data.
- **Self-describing metadata:** start timestamp (with timezone/UTC), device id, firmware version,
  declared sample rate, channel count and order/labels, PGA gain + reference (REF5025 2.5 V) info
  if provided by the board, app version, and a clear scale/units note (counts and, if known, the
  conversion to volts).
- **Integrity:** sequence numbers and explicit **gap markers** for any dropped packets or
  reconnect discontinuities, so analysts can trust timing.
- **Analysis-friendly + interoperable:** importable by common tooling (e.g. Python/NumPy/pandas,
  MATLAB).
- **Annotations travel with the data:** the voice/event annotation track (§5.5,
  `cardiovest.annotations.v1`) is exported alongside the recording and references the same
  timeline, so markers stay aligned to the samples.

Candidate formats to evaluate (decision TBD):

| Candidate | Pros | Cons |
|---|---|---|
| CSV (+ JSON sidecar metadata) | Universal, human-readable, trivial import | Large files, slow, weak for high-rate multichannel |
| Binary samples + JSON/YAML header | Compact, fast, exact | Custom parser needed; must document layout |
| Parquet / Arrow | Columnar, compressed, fast in pandas | Heavier deps; less hand-inspectable |
| HDF5 | Self-describing, scales to large datasets | Heavier deps; mobile tooling maturity varies |
| EDF/EDF+ (biosignal standard) | Established biosignal format, tool support | Constraints/overhead; **must not imply clinical/medical status** |

A pragmatic likely direction (**TBD**): compact binary sample blocks plus a JSON metadata header,
with a CSV/Parquet export option for convenience. Final choice deferred until frame layout (§4.1)
and throughput results (§4.2) are known.

---

## 7. Candidate frameworks (TBD)

Framework choice is **TBD**. The dominant constraints are (a) reliable, low-overhead BLE with MTU /
connection-interval control, and (b) smooth real-time multichannel plotting under sustained data.

| Option | Pros | Cons / risks |
|---|---|---|
| **Flutter (Dart)** | Single codebase iOS+Android; good plotting libs; mature BLE plugins | BLE control depth varies by plugin; high-rate custom rendering may need platform channels |
| **React Native** | Single codebase; large ecosystem | BLE + high-rate streaming/plotting often need native modules; perf tuning effort |
| **Native Android (Kotlin) + Native iOS (Swift)** | Best BLE control + perf; direct GATT/MTU/interval APIs; best rendering | Two codebases; highest effort/cost |
| **Kotlin Multiplatform (KMP)** | Share logic; native BLE per platform | Newer tooling; UI still per-platform or Compose Multiplatform |
| **Single-platform first (e.g. Android only)** | Fastest path to a working research tool; full native BLE | No cross-platform reach until later |

Decision criteria (to resolve TBD): achievable sustained BLE throughput, plotting performance at
target sample rates, team familiarity, and whether one or both platforms are required for the
research program. Recommendation: **prototype the BLE + live-plot path on one platform natively**
to de-risk throughput before committing cross-platform.

---

## 8. Milestones

- **M0 — Foundations**
  - Repo/app skeleton, settings scaffold, BLE permissions wiring, framework spike (§7).
  - Mock data source to develop the plot/recorder without hardware.

- **M1 — BLE connectivity**
  - Scan/filter, connect, GATT discovery, subscribe to (placeholder) sample characteristic.
  - Connection-state UI and manual disconnect. Co-define GATT contract with firmware (§4.1).

- **M2 — Live visualization**
  - Decode real sample frames; scrolling multichannel plot; stream stats (rate/packets/drops).
  - MTU / connection-interval negotiation; measure real throughput (§4.2).

- **M3 — Recording**
  - Start/stop recording from the same ring buffer feeding the plot; local persistence with
    metadata, sequence numbers, and gap markers.

- **M4 — Recordings management + export**
  - Recordings list (rename/metadata/delete); finalize export format (§6); share via OS sheet.

- **M5 — Robustness**
  - Auto-reconnect + backoff, mid-recording gap handling, stall watchdog, backgrounding behavior,
    low-battery/board-shutdown handling, OS BLE edge cases.

- **M6 — Hardening + research UX**
  - Performance tuning at target sample rates, storage management, error states, and prominent,
    persistent **research-only / no-diagnosis** messaging throughout.

*(Milestone sequence is the plan of record; specific dates/owners are TBD.)*

---

## 9. Constraints reminder

The app **shows and records raw research data only** and provides **NO diagnosis,
interpretation, or clinical decision support**. It is a research, prototyping, and education tool —
**not a certified medical device** — and makes **no regulatory claims**.
