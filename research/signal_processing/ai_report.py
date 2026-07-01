#!/usr/bin/env python3
"""
CardioVest -- AI-assisted recording report (proof of concept).

Research / education only. Signal characterization, NOT diagnosis.
See CardioVest/docs/AI_Analysis.md and CardioVest/docs/Safety_Research_Use.md.

Turns the pipeline's reduced outputs (summary_per_minute.csv) into an
AI-written *research* report via the Claude API:

  1. reduce the per-minute summary to a compact per-HOUR overview
     (raw samples are NEVER sent to the model),
  2. ask Claude for a plain-language summary + a structured list of hour
     windows to FLAG FOR RESEARCH REVIEW (neutral, engineering wording),
  3. save out/ai_report.md and out/ai_report.json.

--dry-run builds and prints the exact prompt without calling the API
(no key, no `anthropic` package needed) so the reduction + framing can be
reviewed offline. A real run needs ANTHROPIC_API_KEY.

Usage:
  python ai_report.py --dry-run                       # preview prompt, no API
  python ai_report.py                                 # real run (needs key)
  python ai_report.py --summary out/summary_per_minute.csv --model claude-opus-4-8
"""
import argparse, csv, json, os, sys
from collections import defaultdict

MODEL_DEFAULT = "claude-opus-4-8"     # most capable; --model to override
MINUTES_PER_HOUR = 60
MAX_TOKENS = 12000                    # < ~16k, safe non-streaming; headroom for thinking + JSON

# Research-only framing. The model must describe DATA, never diagnose.
SYSTEM_PROMPT = (
    "You are a signal-analysis assistant for CardioVest / CardioCore V1, an "
    "experimental, NON-MEDICAL ECG research platform.\n"
    "STRICT SCOPE -- research and engineering only:\n"
    "- This is NOT a medical device and the data is EXPERIMENTAL only.\n"
    "- Do NOT diagnose, screen for, or name any medical condition. Do NOT give "
    "medical advice, reassurance, or warnings.\n"
    "- Describe the DATA in neutral engineering terms (e.g. 'atypical heart-rate "
    "stretch', 'high-noise window', 'low beat count'), never clinical terms.\n"
    "- Everything you surface is 'flagged for research review' by a human "
    "engineer -- never a conclusion.\n"
    "You are given a REDUCED, per-hour overview of a long recording (never raw "
    "samples). Point out stretches that stand out numerically and are worth a "
    "human's closer look."
)

# Structured output: guarantees a machine-readable flag list.
FLAG_SCHEMA = {
    "type": "object",
    "additionalProperties": False,
    "properties": {
        "summary": {
            "type": "string",
            "description": "Short, neutral, engineering description of the recording overview (2-5 sentences).",
        },
        "flagged_windows": {
            "type": "array",
            "description": "Hour windows that stand out numerically and are worth a human research review.",
            "items": {
                "type": "object",
                "additionalProperties": False,
                "properties": {
                    "start_hour": {"type": "integer"},
                    "end_hour": {"type": "integer"},
                    "reason": {"type": "string", "description": "Neutral engineering reason (e.g. 'high %noise', 'HR far from baseline'). No clinical language."},
                    "metric": {"type": "string", "description": "Numeric evidence, e.g. 'mean_hr=131, noise=82%'."},
                },
                "required": ["start_hour", "end_hour", "reason", "metric"],
            },
        },
    },
    "required": ["summary", "flagged_windows"],
}

DISCLAIMER = (
    "Research / education only -- CardioVest is NOT a medical device. This report "
    "characterizes experimental data and is NOT diagnosis. Flags are for human "
    "research review only. See CardioVest/docs/Safety_Research_Use.md."
)

# Output-side backstop to the prompt guardrail: refuse to emit clinical wording
# even if the model produced it. Starting denylist (extend as needed).
CLINICAL_TERMS = (
    "diagnos", "arrhythm", "fibrillation", "flutter", "tachycard", "bradycard",
    "ectopic", "ventricul", "atrial", "palpitat", "syncop", "murmur", "ischem",
    "ischaem", "infarct", "heart block", "heart attack", "st eleva", "qtc",
    "afib", "a-fib", "cardiolog", "physician", "prescrib", "medication",
    "disease", "patholog",
)


def screen_clinical(result):
    """Fail closed: never write a report that contains clinical/diagnostic terms."""
    hits = sorted({t for t in CLINICAL_TERMS if t in json.dumps(result).lower()})
    if hits:
        sys.exit(f"error: model output contained clinical/diagnostic wording {hits} -- "
                 "refusing to write a report. This tool is research-only (not diagnosis).")


def _f(x):
    try:
        return float(x)
    except (TypeError, ValueError):
        return 0.0


def load_summary(path):
    with open(path, newline="") as f:
        rows = list(csv.DictReader(f))
    if not rows:
        sys.exit(f"error: {path} has no rows")
    return rows


def reduce_to_hours(rows):
    """Roll the per-minute summary (minute,n_beats,mean_hr,sdnn_ms,sqi) up to
    one compact row per hour. This is the ONLY thing sent to the model."""
    by_hour = defaultdict(list)
    for r in rows:
        by_hour[int(_f(r.get("minute", 0))) // MINUTES_PER_HOUR].append(r)
    overview = []
    for h in sorted(by_hour):
        mins = by_hour[h]
        n = len(mins)
        hrs = [_f(r.get("mean_hr")) for r in mins if _f(r.get("mean_hr")) > 0]
        sdnns = [_f(r.get("sdnn_ms")) for r in mins if _f(r.get("sdnn_ms")) > 0]
        sqi = [r.get("sqi", "-") for r in mins]
        noisy, flat = sqi.count("noisy"), sqi.count("flat")
        overview.append({
            "hour": h,
            "minutes": n,
            "beats": sum(int(_f(r.get("n_beats"))) for r in mins),
            "mean_hr": round(sum(hrs) / len(hrs), 1) if hrs else 0.0,
            "min_hr": round(min(hrs), 1) if hrs else 0.0,
            "max_hr": round(max(hrs), 1) if hrs else 0.0,
            "mean_sdnn_ms": round(sum(sdnns) / len(sdnns), 1) if sdnns else 0.0,
            "noise_pct": round(100.0 * (noisy + flat) / n) if n else 0,
            "good_min": sqi.count("good"), "noisy_min": noisy, "flat_min": flat,
        })
    return overview


def build_user_message(overview):
    return (
        f"Here is the per-hour overview of one CardioCore V1 recording "
        f"({len(overview)} hour(s)). Each row: hour index, minutes covered, total "
        f"beats, mean/min/max estimated HR (bpm), mean SDNN (ms), % of minutes "
        f"tagged noisy or flat, and good/noisy/flat minute counts. All values are "
        f"DSP estimates on EXPERIMENTAL data.\n\n"
        f"Give: (1) a short neutral summary of the recording, and (2) the hour "
        f"windows that stand out numerically and deserve a human research review, "
        f"with the numeric evidence. Engineering wording only -- no clinical "
        f"terms, no diagnosis.\n\n"
        f"OVERVIEW (JSON):\n{json.dumps(overview)}"
    )


def call_claude(model, system, user_msg):
    try:
        import anthropic
    except ImportError:
        sys.exit("error: the 'anthropic' package is required (pip install anthropic), or use --dry-run.")
    if not os.environ.get("ANTHROPIC_API_KEY"):
        sys.exit("error: set ANTHROPIC_API_KEY (or use --dry-run to preview the prompt without an API call).")

    kwargs = dict(
        model=model,
        max_tokens=MAX_TOKENS,
        system=system,
        messages=[{"role": "user", "content": user_msg}],
        output_config={"format": {"type": "json_schema", "schema": FLAG_SCHEMA}},
    )
    if "haiku" not in model:                    # adaptive thinking + capped effort on Opus/Sonnet 4.6+, not Haiku
        kwargs["thinking"] = {"type": "adaptive"}
        kwargs["output_config"]["effort"] = "medium"

    resp = anthropic.Anthropic().messages.create(**kwargs)
    if resp.stop_reason == "refusal":
        sys.exit("error: request was declined by safety classifiers (stop_reason=refusal).")
    if resp.stop_reason == "max_tokens":
        sys.exit("error: response hit the max_tokens cap before finishing -- raise MAX_TOKENS or lower effort.")
    text = next((b.text for b in resp.content if b.type == "text"), "")
    try:
        return json.loads(text), resp.usage
    except json.JSONDecodeError:
        sys.exit(f"error: model output was not valid JSON (stop_reason={resp.stop_reason}). Raw:\n{text[:500]}")


def write_report(outdir, result, model):
    os.makedirs(outdir, exist_ok=True)
    with open(os.path.join(outdir, "ai_report.json"), "w") as f:
        json.dump({"disclaimer": DISCLAIMER, "research_only": True, "model": model, **result}, f, indent=2)

    lines = [
        "# CardioVest -- AI recording report (research only)",
        "", f"> {DISCLAIMER}", "", f"_Model: {model}_", "",
        "## Summary", "", result.get("summary", "").strip(), "",
        "## Flagged windows (for research review)", "",
    ]
    fw = result.get("flagged_windows", [])
    if fw:
        lines += ["| hours | reason | evidence |", "|---|---|---|"]
        lines += [f"| {w.get('start_hour')}–{w.get('end_hour')} | {w.get('reason', '')} | {w.get('metric', '')} |" for w in fw]
    else:
        lines.append("_None flagged._")
    lines.append("")
    with open(os.path.join(outdir, "ai_report.md"), "w") as f:
        f.write("\n".join(lines))


def main():
    ap = argparse.ArgumentParser(description="CardioVest AI recording report (research only, not diagnosis).")
    ap.add_argument("--summary", default=os.path.join("out", "summary_per_minute.csv"),
                    help="per-minute summary CSV from analyze_recording.py (default out/summary_per_minute.csv)")
    ap.add_argument("--model", default=MODEL_DEFAULT, help=f"Claude model id (default {MODEL_DEFAULT})")
    ap.add_argument("--out", default="out", help="output directory (default out)")
    ap.add_argument("--dry-run", action="store_true", help="print the prompt without calling the API (no key needed)")
    a = ap.parse_args()

    if not os.path.exists(a.summary):
        sys.exit(f"error: {a.summary} not found. Run analyze_recording.py first (see README).")

    overview = reduce_to_hours(load_summary(a.summary))
    if not overview:
        sys.exit("error: no usable minutes in the summary.")
    user_msg = build_user_message(overview)
    print(f"Reduced summary -> {len(overview)} hourly row(s) (raw samples never sent).")

    if a.dry_run:
        print("\n===== SYSTEM =====\n" + SYSTEM_PROMPT)
        print("\n===== USER =====\n" + user_msg)
        print("\n(--dry-run: no API call made)")
        return

    result, usage = call_claude(a.model, SYSTEM_PROMPT, user_msg)
    screen_clinical(result)
    write_report(a.out, result, a.model)
    print("\n" + result.get("summary", "").strip() + "\n")
    print(f"Flagged {len(result.get('flagged_windows', []))} window(s). -> {a.out}/ai_report.md, {a.out}/ai_report.json")
    if usage:
        print(f"tokens: in={usage.input_tokens} out={usage.output_tokens}")


if __name__ == "__main__":
    main()
