# CardioVest — Long-Recording Data Pipeline

> How to store and analyze **long, continuous** ECG recordings (target: ~15 days of wear).
> **Research / education only** — this is signal characterization and engineering analysis,
> **not** diagnosis or clinical interpretation.

## 1. The scale of the problem

Continuous 8-channel acquisition (transport frame = `[u32 seq][status 3B][8 × 3B]` = 31 B):

| Rate | Per second | Per day | 15 days | Time-points (15 d) | Beats (~72 bpm) |
|---|---|---|---|---|---|
| 500 SPS | 15.5 KB/s | ~1.3 GB | **~20 GB** | ~650 million | ~1.5 million |
| 250 SPS | 7.8 KB/s | ~0.67 GB | **~10 GB** | ~325 million | ~1.5 million |

A 32–64 GB microSD stores it easily. **The challenge is analysis, not storage** — you cannot load
20 GB / billions of samples into memory. Everything below follows one rule:

> **Reduce, don't load. Archive the raw; analyze compact derived layers; open raw only on demand.**

## 2. Data layers

The pipeline turns the huge continuous signal into small, useful layers. Analysis and
visualization run on L1–L4 (megabytes); L0 is touched only for flagged strips.

| Layer | What | Size (15 d) | Purpose |
|---|---|---|---|
| **L0 — Raw** | The continuous binary log (`.bin` on the card) | ~10–20 GB | Archive / source of truth; rarely read whole |
| **L1 — Quality (SQI)** | Per-window tag: good / noisy / flat | ~KB | Skip/flag artifact (a lot of 15-day wear is motion) |
| **L2 — Beats** | Per-beat: time, RR interval, features | a few MB | HR, HRV, rhythm analysis without the raw |
| **L3 — Summaries** | Per-minute: mean HR, HRV, noise %, beat count | ~tiny (21,600 rows) | Trends over days; the overview timeline |
| **L4 — Events / strips** | Full-resolution raw **snippets** around markers, candidate events, noisy stretches | small | Human/analyst drill-down |

This mirrors how ambulatory patches (e.g. Zio) scale: a beat/annotation stream + rhythm episodes
+ full strips only where something is flagged. See also [Roadmap.md](Roadmap.md) (Zio-inspired ideas).

## 3. Storage & format

- **On device (firmware):** just log **raw** continuously to microSD (already done). No on-device
  analysis. Optionally, later: on-device downsampling/compression for constrained cases (TBD).
- **Ingest (offload → convert):** copy the card, then convert `.bin` → a **chunked, columnar**
  format for analysis:
  - **Parquet** (columnar, compressed, fast partial/range reads via `pyarrow`), or **HDF5**
    (chunked, standard for biosignals via `h5py`). ECG compresses well (delta + general compression).
  - **Partition by time** (e.g. one file per hour → 360 files for 15 d) so you can seek to a range
    and **process files in parallel**.
  - Carry the **`sample_index`** (our per-frame counter) so timing is exact and **gaps are explicit**
    (seq jumps = dropped/again-missing frames), not guessed.

## 4. Processing model

- **Streaming / chunked:** never read the whole file. Process in blocks (this pipeline works
  **minute-by-minute**, ~30k samples/min at 500 SPS → constant memory), with a small overlap
  between blocks for filter warm-up and boundary beats.
- **Parallel:** because it's offline and time-partitioned, chunks/hours can be processed
  concurrently (`multiprocessing` / `dask`) and the per-minute summaries merged.
- **Vectorized:** filtering and peak-finding operate on numpy arrays per block, not sample-by-sample.

### Stages
```
L0 raw (.bin on card)
   └─▶ ingest/convert  → time-partitioned Parquet/HDF5   (+ gap map from seq)
          └─▶ reduce   → SQI per window (L1) + R-peak/beat detection (L2)
                 └─▶ summarize → per-minute HR / HRV / noise% (L3)
                        └─▶ overview timeline + drill-down into raw strips (L4)
```

## 5. Tools (Python)

- `numpy` (chunked math), `pyarrow`/`pandas` (Parquet), `h5py` (HDF5), `scipy.signal` (filters).
- ECG-specific research libraries: **NeuroKit2**, **wfdb**, **BioSPPy** (R-peak, HRV) — optional,
  for richer analysis; the first pipeline uses a self-contained numpy detector to stay dependency-light.
- `dask` for out-of-core / parallel processing of the partitioned dataset.

## 6. Visualizing 15 days

Don't scroll 15 days of waveform. Provide:
- A **whole-recording overview**: HR trend + HRV + **noise/quality bands** + **markers**, across days
  (from L3/L1) — a few thousand points, instant to render.
- **Click-to-drill-down:** clicking a time/event loads just that **raw strip** (L4/L0 range read).

This is the same shape as the web viewer's live plot, but fed by the reduced layers with on-demand
raw access — a natural next iteration of `software/`.

## 7. How it maps to this repo

- **Firmware** already writes L0 (raw transport frames) to microSD, each stamped with `sample_index`.
- **Web viewer** prototypes L1 (SQI) and L2 (R-peak/BPM) live; those algorithms move into the
  offline pipeline for scale.
- **Markers / annotations** (`cardiovest.annotations.v1`) feed L4 (events of interest).
- **`research/signal_processing/`** hosts the offline pipeline (see `analyze_recording.py`).

## 8. Open questions (TBD)
- Final storage format (Parquet vs HDF5) and partition size (per-hour vs per-N-minutes).
- On-device compression / adaptive sample rate for very long / low-power runs.
- Beat-feature set to store in L2 (RR only vs template/morphology).
- Channel strategy for the wearable (all 8 vs a reduced lead set) — directly changes the volume.
