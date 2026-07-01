# CardioVest — AI-Assisted Analysis (Claude API)

> How to use an LLM (the **Claude API**) to help analyze long CardioCore V1 recordings —
> triaging the reduced data layers, describing patterns, and generating research reports.
> **Research / education only.** This is an engineering / research aid, **not** diagnosis or
> clinical interpretation. See [Safety_Research_Use.md](Safety_Research_Use.md) — the single
> source of truth for scope and safety. Nothing here detects, screens for, or rules out any
> condition, and no AI output may inform a health decision.

This builds directly on [Data_Pipeline.md](Data_Pipeline.md): the DSP pipeline reduces a huge
recording into small layers (L1–L4); the LLM reasons over **those** layers, never the raw signal.

## 1. The core rule — the LLM never reads the raw data

The ~20 GB / ~650 million samples of a 15-day recording do **not** go to the Claude API — they
neither fit nor are they what a language model is good at. Raw ADC counts are a job for **DSP**,
not for language.

The model reads the **compact products the pipeline already emits** (L1–L4 from
[Data_Pipeline.md](Data_Pipeline.md)) — per-minute summaries, beat/RR tables, SQI labels, and only
the full-resolution *strips* of the handful of flagged moments. That is **megabytes, not
gigabytes** — small enough to send, and exactly where an LLM is strong.

```
L0 raw (~20 GB)          ── NEVER sent to the API
   └─ pipeline (DSP)  → L1/L2/L3 reduced products (MB)   ── this is what Claude reads
        └─ Claude API → triage / describe / report / decide where to look
             └─ requests one strip (L4) → your code returns just that window
```

> **Reduce, then reason.** The DSP layer is what makes AI analysis of a 15-day wear both
> *possible* and *cheap* — the model works on summaries, not on billions of samples.

## 2. Division of labor — who does what

| Job | Tool | Why |
|---|---|---|
| **Find & measure** (R-peaks, RR, HR/HRV, SQI, %noise) | Deterministic DSP — the current pipeline (`analyze_recording.py`) | Fast, reproducible, cheap, auditable |
| **Classify beat *morphology*** (waveform shape, research only) | A **specialized ML model** (1D CNN) — *future phase* | A DSP/ML pattern-recognition job, **not** an LLM job. It would characterize waveform *shape* for research only and would **never** output a condition or diagnosis. |
| **Triage / summarize / describe / report / decide the next look** | **Claude API** | "Search the data for something notable" = read the reduced layers and point at what merits a human look |

The LLM sits **above** the numeric layer, orchestrating and explaining. It is deliberately **not**:

- the primary R-peak / beat-morphology detector from the raw signal (that is DSP / specialized ML), or
- a diagnostic engine of any kind (see §7).

## 3. Where AI plugs into the pipeline

The pipeline's outputs are the LLM's inputs. Concretely, today's products (from
`research/signal_processing/analyze_recording.py`):

| Product | Columns | LLM use |
|---|---|---|
| `summary_per_minute.csv` (L3) | `minute, n_beats, mean_hr, sdnn_ms, sqi` | The overview timeline — the model reads it (aggregated) to find atypical stretches |
| `beats.csv` (L2) | `t_s, rr_ms, bpm` | RR/HR detail for a flagged window (never the whole 15 days at once) |
| SQI labels (L1) | `good / noisy / flat` per minute | Tell the model which minutes to trust vs. skip as artifact |
| Strips (L4) | raw snippet around a marker / candidate | Fetched **on demand** for the few moments the model wants to look at closely |

Everything the model sees is compact JSON/CSV text. The raw `.bin` (L0) is opened only for a
specific strip, and only when asked for.

## 4. How the model "searches" — the agentic pattern

To let Claude *vasculhar* (sweep) the recording, give it **tools** that query the reduced layers,
and let it iterate (tool use / agentic loop):

1. `get_overview()` → returns the recording rolled up to **per-hour** (≈360 rows for 15 days):
   HR trend, HRV, %noise. The model spots the hours that stand out.
2. `get_minutes(hour)` → returns the per-minute rows for one hour (from L3).
3. `get_strip(t)` → returns the raw window at `t` (an L4/L0 range read) — the only path to raw,
   and only for a specific instant.
4. The model writes a **research report** describing what is notable and lists the timestamps to
   **flag for research review**.

The loop keeps the context tiny: the model pulls detail only where its own overview pass found
something, so it never ingests the full billion-sample stream.

**Two passes, cheap first:** a broad, cheap triage pass over the whole overview (small model),
then a focused, deeper pass only on the flagged windows (stronger model). See §5–§6.

## 5. The Claude API in practice

Concrete, current API facts (see the `claude-api` skill / `platform.claude.com` for the latest):

- **Models & IDs.** Default to **`claude-opus-4-8`** (most capable) for the deep pass. For the
  **bulk** triage over thousands of minutes, **`claude-haiku-4-5`** is cheap/fast; escalate to Opus
  only on the interesting windows. (`claude-sonnet-4-6` is the middle option.) Use the exact ID
  strings — no date suffixes.
- **Thinking.** On Opus 4.8 use adaptive thinking (`thinking: {"type": "adaptive"}`) for the
  reasoning-heavy report pass; control depth with `output_config: {"effort": "..."}`. Do **not**
  use `budget_tokens` (removed on 4.8).
- **Batch API** — the natural fit for an offline 15-day job: asynchronous and **~50% cheaper**.
  Submit every hour/window as one batch, poll, then collect. (`client.messages.batches.create`.)
- **Prompt caching** — cache the fixed context (instructions + "what each metric means") once;
  repeats read it at ~0.1× cost. Keep the volatile per-window data *after* the cached prefix.
- **Structured output** — constrain the model to a JSON schema (`output_config.format`, or
  `client.messages.parse()` with a Pydantic model) so the flagged-windows list is machine-readable,
  not free text to parse.
- **Streaming** — use it for the long report pass to avoid request timeouts.

## 6. Cost sketch (15 days)

Because the DSP layer reduces the data first, AI analysis of the whole wear is inexpensive:

| Pass | Input to the model | Rough size | Rough cost |
|---|---|---|---|
| Overview triage | `summary` rolled up to per-hour (~360 rows) | tens of k tokens | fractions of a cent |
| Full per-minute triage (if wanted) | all ~21,600 minute rows once | ~a few hundred k tokens | ~US$0.20–2 (Haiku↔Opus); **half that with Batch** |
| Deep dive | only the flagged windows' beats/strips | small | cents |

Order-of-magnitude: the **entire** 15-day AI triage is **cents to a couple of dollars** — the point
is that the pipeline shrinks the data *before* the model ever sees it.

## 7. The guardrail (non-negotiable)

This is **research / engineering**, not medicine. The AI layer here:

- **describes, triages, and explains** *your own experimental data* — it does **not** diagnose;
- frames every finding as **"flagged for research review"**, never as a clinical conclusion;
- must never be used for diagnosis, screening, treatment, patient monitoring, emergency use, or any
  clinical decision — per [Safety_Research_Use.md](Safety_Research_Use.md).

Practical rules for any AI-analysis code/prompt in this repo:

- The **system prompt states the research-only framing** and forbids diagnostic/clinical language.
- Outputs carry the same disclaimer and use neutral, engineering wording ("atypical HR stretch",
  "high-noise window") — never condition names or clinical claims.
- No fabricated regulatory status. No advice, no reassurance, no alarming — just characterization.

## 8. How it maps to this repo

- **Pipeline** (`research/signal_processing/analyze_recording.py`) produces L1–L3 (the model's input).
- **Markers / annotations** (`cardiovest.annotations.v1`, from the web viewer) feed L4 (events of
  interest) — natural anchors for the model to describe.
- **This doc's PoC** is `research/signal_processing/ai_report.py` (see §9): pipeline CSVs → Claude API
  → a natural-language recording report + a flagged-windows list.
- The **web viewer** (`software/`) is the natural place to later surface the report + drill-down.

## 9. Proof of concept — `ai_report.py`

A dependency-light script that turns the pipeline's output into an AI-written research report. It
is **single-shot and overview-only** — no tool use, no beats, no raw strips (the drill-down tools
in §3–§4 are the future agentic design, not this script):

- **Input:** `summary_per_minute.csv` from a pipeline run.
- **Reduce for the model:** roll the per-minute summary up to a **per-hour** overview — the *only*
  thing placed in the prompt. Raw samples, `beats.csv`, and strips are never sent. Each hour row is:
  `hour, minutes, beats, mean/min/max HR, mean SDNN, noise %, good/noisy/flat minute counts`.
- **Ask Claude** (official `anthropic` Python SDK): a research-only system prompt + the compact
  overview → a plain-language summary and a **structured list of windows to flag for research
  review** (neutral reason + numeric evidence for each). It uses **structured output**; Batch,
  prompt caching, and streaming (§5) are future optimizations, not wired into the PoC.
- **Guardrail baked in (defense in depth):** research-only system prompt, neutral wording, an
  output-side clinical-term screen that **fails closed**, and the disclaimer written into *both* the
  `.md` and `.json` artifacts.
- **Runs without a key:** `--dry-run` builds and prints the exact prompt (no API call) so the
  reduction and framing can be reviewed offline. A real run needs `ANTHROPIC_API_KEY`.

It is a *starting point*, not the final analyzer — the point is to prove the "reduce → reason"
loop end-to-end on real pipeline output.

## 10. Open questions (TBD)

- Overview granularity for the triage pass (per-hour vs per-10-min) vs. cost/recall trade-off.
- Batch vs. interactive for routine runs; when to escalate Haiku → Opus per window.
- Tool-use agent (§4) vs. single-shot report — start single-shot, add tools when drill-down is needed.
- What structured schema best captures a "flagged window" for the viewer's drill-down.
- Whether a specialized beat-morphology model (§2) is worth adding beneath the LLM layer.
