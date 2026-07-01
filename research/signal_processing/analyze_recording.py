#!/usr/bin/env python3
"""
CardioVest — long-recording analysis pipeline (v1).

Research / education only. Signal characterization, NOT diagnosis.

Reduces a long ECG recording into compact layers (see docs/Data_Pipeline.md):
  L1 signal-quality (SQI) per minute, L2 beats (R-peaks -> RR/HR), L3 per-minute
  summary. Processes MINUTE-BY-MINUTE so memory stays constant regardless of
  length (built for ~15-day recordings).

Inputs:
  --synthetic MIN   generate MIN minutes of synthetic ECG in memory (no file) -- default 5
  --input FILE      read a raw firmware log (transport frames: [u32 seq][3B status][N*3B])
  --fs, --channels, --analysis-ch, --out

Outputs (in --out dir):
  beats.csv              t_s, rr_ms, bpm
  summary_per_minute.csv minute, n_beats, mean_hr, sdnn_ms, sqi
  overview.png           HR trend + noisy minutes (if matplotlib is available)
"""
import argparse, csv, os, sys, math
import numpy as np

REC_OVERHEAD = 4 + 3            # u32 sample_index + 24-bit status
VREF, PGA = 2.5, 6.0           # counts <-> mV (matches firmware demo)


# --------------------------------------------------------------------------- #
#  Sources (yield one minute at a time: (minute_index, seq0, data[nch, n]))
# --------------------------------------------------------------------------- #
def source_synth(minutes, fs, nch):
    rng = np.random.default_rng(1)
    for m in range(minutes):
        n = fs * 60
        t = m * 60.0 + np.arange(n) / fs
        hr = 72.0 + 10.0 * math.sin(2 * math.pi * m / max(1, minutes))     # gentle HR drift
        ph = (t * (hr / 60.0)) % 1.0
        g = lambda c, w, a: a * np.exp(-((ph - c) ** 2) / (2 * w * w))
        mv = (g(0.20, 0.025, 0.10) - g(0.38, 0.008, 0.13) + g(0.40, 0.011, 1.0)
              - g(0.43, 0.010, 0.28) + g(0.62, 0.040, 0.30))
        mv = mv + 0.03 * np.sin(2 * math.pi * 0.25 * t)                     # baseline wander
        data = np.empty((nch, n))
        for c in range(nch):
            amp = 0.55 + 0.5 * abs(math.sin(1.3 * c + 0.6))
            data[c] = (mv * amp + 0.02 * rng.standard_normal(n)) * 1e-3 * PGA / VREF * 8388607.0
        if minutes >= 5 and m % 5 == 4:                                    # inject a noisy minute
            data += 40000.0 * rng.standard_normal((nch, n))
        yield m, m * fs * 60, data


def source_bin(path, fs, nch):
    rec = REC_OVERHEAD + nch * 3
    fpm = fs * 60
    with open(path, "rb") as f:
        m = 0
        while True:
            buf = f.read(rec * fpm)
            nfr = len(buf) // rec
            if nfr == 0:
                break
            a = np.frombuffer(buf[:nfr * rec], dtype=np.uint8).reshape(nfr, rec)
            data = np.empty((nch, nfr))
            for c in range(nch):
                o = REC_OVERHEAD + c * 3
                v = (a[:, o].astype(np.int32) << 16) | (a[:, o + 1].astype(np.int32) << 8) | a[:, o + 2].astype(np.int32)
                data[c] = np.where(v & 0x800000, v - 0x1000000, v)
            seq0 = int(a[0, 0]) | (int(a[0, 1]) << 8) | (int(a[0, 2]) << 16) | (int(a[0, 3]) << 24)
            yield m, seq0, data
            m += 1


# --------------------------------------------------------------------------- #
#  DSP (numpy only)
# --------------------------------------------------------------------------- #
def _ma(x, w):
    w = max(1, int(w))
    return x if w < 2 else np.convolve(x, np.ones(w) / w, mode="same")


def baseline_removed(x, fs):
    return x.astype(np.float64) - _ma(x.astype(np.float64), fs * 0.6)      # ~HP, removes wander


def detect_beats(sig, fs, core=0):
    """Return R-peak sample indices in `sig` (Pan-Tompkins-lite).

    `core` = index where the current minute starts inside `sig` (the rest is the
    carried-over overlap). The threshold uses the median energy of the CURRENT
    minute only, so a noise spike in the overlap tail can't poison it.
    """
    b = baseline_removed(sig, fs)
    d = np.diff(b, prepend=b[0])
    energy = _ma(d * d, fs * 0.03)                                          # derivative^2, integrated
    if energy.size == 0:
        return []
    ref = energy[core:] if core < energy.size else energy
    thr = 5.0 * np.median(ref) if ref.size else 0.0                        # robust to spikes (median, not max)
    if thr <= 0:
        return []
    inner = energy[1:-1]
    cand = np.where((inner > thr) & (inner >= energy[:-2]) & (inner >= energy[2:]))[0] + 1
    refr, peaks, last = int(0.25 * fs), [], -10 ** 9
    for p in cand:                                                          # enforce refractory (sparse)
        if p - last >= refr:
            peaks.append(int(p)); last = p
    return peaks


def sqi_label(sig, fs):
    b = baseline_removed(sig, fs)
    p2p = np.percentile(b, 99) - np.percentile(b, 1)
    if p2p < 300:
        return "flat"
    # high-frequency energy ratio: ~2 for broadband/motion noise, <~0.3 for clean
    # ECG. Amplitude-independent, so it catches big noise that p2p alone misses.
    hf_ratio = np.mean(np.diff(b) ** 2) / (np.mean(b ** 2) + 1e-9)
    return "noisy" if hf_ratio > 0.4 else "good"


# --------------------------------------------------------------------------- #
#  Pipeline
# --------------------------------------------------------------------------- #
def run(source, fs, ach, outdir):
    os.makedirs(outdir, exist_ok=True)
    beats_abs, sqi_by_min = [], {}
    tail = np.empty(0)
    ov = int(fs * 2)                                                        # 2 s overlap for continuity
    total = 0
    for m, _seq0, block in source:
        an = block[ach]
        seg = np.concatenate([tail, an]) if tail.size else an
        core = seg.size - an.size                                          # where the current minute starts in seg
        seg_start = total - core
        for p in detect_beats(seg, fs, core):
            if p >= core:
                beats_abs.append(seg_start + p)                            # de-dup: only beats inside the minute
        sqi_by_min[m] = sqi_label(an, fs)
        tail = an[-ov:] if an.size >= ov else an
        total += an.size
        if (m + 1) % 60 == 0:
            print(f"  processed {m + 1} min ({total / fs / 3600:.1f} h), beats={len(beats_abs)}")

    beats_abs.sort()
    beat_t = np.array(beats_abs) / fs

    # L2 beats.csv
    with open(os.path.join(outdir, "beats.csv"), "w", newline="") as f:
        w = csv.writer(f); w.writerow(["t_s", "rr_ms", "bpm"])
        for i in range(len(beat_t)):
            rr = (beat_t[i] - beat_t[i - 1]) * 1000.0 if i else 0.0
            bpm = f"{60000.0 / rr:.1f}" if 300.0 <= rr <= 2000.0 else ""    # blank if non-physiological (gap)
            w.writerow([f"{beat_t[i]:.3f}", f"{rr:.1f}", bpm])

    # L3 per-minute summary
    n_min = total // (fs * 60) + (1 if total % (fs * 60) else 0)
    rr_by_min = {}
    for i in range(1, len(beat_t)):
        rr = (beat_t[i] - beat_t[i - 1]) * 1000.0
        if 300.0 <= rr <= 2000.0:                                          # reject non-physiological RR (gaps/artifact)
            rr_by_min.setdefault(int(beat_t[i] // 60), []).append(rr)
    beats_by_min = {}                                                      # true detected-beat count per minute
    for t in beat_t:
        mm = int(t // 60)
        beats_by_min[mm] = beats_by_min.get(mm, 0) + 1
    rows = []
    for m in range(n_min):
        rr = rr_by_min.get(m, [])
        mean_hr = 60000.0 / np.mean(rr) if rr else 0.0
        sdnn = float(np.std(rr)) if len(rr) > 1 else 0.0
        rows.append([m, beats_by_min.get(m, 0), round(mean_hr, 1), round(sdnn, 1), sqi_by_min.get(m, "-")])
    with open(os.path.join(outdir, "summary_per_minute.csv"), "w", newline="") as f:
        w = csv.writer(f); w.writerow(["minute", "n_beats", "mean_hr", "sdnn_ms", "sqi"]); w.writerows(rows)

    noisy = sum(1 for r in rows if r[4] != "good")
    print(f"\nDone: {len(beat_t)} beats, {n_min} minutes, noise={100.0 * noisy / max(1, n_min):.0f}%")
    print(f"  -> {outdir}/beats.csv, summary_per_minute.csv")
    _overview(rows, outdir)


def _overview(rows, outdir):
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except Exception:
        print("  (matplotlib not available -> skipping overview.png)")
        return
    mins = [r[0] for r in rows]; hr = [r[2] for r in rows]
    fig, ax = plt.subplots(figsize=(10, 3.2))
    ax.plot(mins, hr, color="#46d39a", lw=1.2, label="mean HR (est.)")
    for r in rows:                                                         # shade non-good minutes
        if r[4] != "good":
            ax.axvspan(r[0] - 0.5, r[0] + 0.5, color="#ffcf5c", alpha=0.25)
    ax.set_xlabel("minute"); ax.set_ylabel("BPM (est.)")
    ax.set_title("CardioVest — recording overview (research; not diagnosis)")
    ax.legend(loc="upper right", fontsize=8)
    fig.tight_layout(); fig.savefig(os.path.join(outdir, "overview.png"), dpi=110)
    print(f"  -> {outdir}/overview.png")


def main():
    ap = argparse.ArgumentParser(description="CardioVest long-recording analysis (research only).")
    ap.add_argument("--input", help="raw firmware log (.bin transport frames)")
    ap.add_argument("--synthetic", type=int, default=5, help="generate N minutes of synthetic ECG (default 5)")
    ap.add_argument("--fs", type=int, default=500, help="sample rate (Hz)")
    ap.add_argument("--channels", type=int, default=8, help="channel count")
    ap.add_argument("--analysis-ch", type=int, default=0, help="channel index for beat detection")
    ap.add_argument("--out", default="out", help="output directory")
    a = ap.parse_args()
    if a.input:
        print(f"Reading {a.input} (fs={a.fs}, ch={a.channels})")
        src = source_bin(a.input, a.fs, a.channels)
    else:
        print(f"Synthetic: {a.synthetic} min, fs={a.fs}, ch={a.channels}")
        src = source_synth(a.synthetic, a.fs, a.channels)
    run(src, a.fs, min(a.analysis_ch, a.channels - 1), a.out)


if __name__ == "__main__":
    main()
