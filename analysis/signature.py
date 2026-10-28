#!/usr/bin/env python3
"""Second pass: characteristic EQ curve (octave bands), recurring resonances,
similarity clustering, and per-track fingerprints -> readable report tables."""
import os, json, glob
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

OUT = "analysis/out"
FIG = "analysis/figs"
files = sorted(glob.glob(os.path.join(OUT, "*.json")))

TRACKS = [json.load(open(f)) for f in files]

# ---------- 1. octave-band signature curve (K-weighting free, energy-normalised)
OCT = [(20, 40), (40, 80), (80, 160), (160, 320), (320, 640), (640, 1280),
       (1280, 2560), (2560, 5120), (5120, 10240), (10240, 20480)]
LABELS = ["20-40", "40-80", "80-160", "160-320", "320-640", "640-1.2k",
          "1.2-2.5k", "2.5-5k", "5-10k", "10-20k"]

curve = []
for t in TRACKS:
    f = np.array(t["spectral"]["spectrum_freqs"])
    s = np.array(t["spectral"]["spectrum_db"])
    row = []
    for lo, hi in OCT:
        m = (f >= lo) & (f < hi)
        row.append(float(np.mean(s[m])) if m.any() else np.nan)
    curve.append(row)
C = np.array(curve)
med = np.median(C, axis=0)

# relative to a 4.5 dB/oct "natural music" slope anchored at 1 kHz
k_idx = 5  # 640-1.2k
slope = np.array([4.5 * np.log2(max(np.sqrt(lo * hi), 1) / 1000) for lo, hi in OCT])
rel = med - (med[k_idx] - slope[k_idx] * 0 - 0) + 0
rel = med - med[k_idx] + (0) - (slope - slope[k_idx])  # tilt-corrected shape
rel = med - med[k_idx]

print("== octave band levels (dB re 1 kHz band, median of 22) ==")
for l, v, r in zip(LABELS, med, rel):
    print(f"  {l:>9}: {v - med[k_idx]:+6.1f} dB")

# ---------- 2. recurring resonances
from collections import Counter
bins = Counter()
all_res = []
for t in TRACKS:
    rr = t["spectral"].get("resonances") or []
    all_res.append(rr)
    for p in rr:
        if p["excess_db"] >= 6:
            bins[round(np.log2(p["f"]) * 12)] += 1   # 1/12-octave bins
print("\n== recurring spectral peaks (1/12-oct bins hit in >=5 tracks) ==")
hot = [(2 ** (k / 12), n) for k, n in bins.items() if n >= 5]
for hz, n in sorted(hot):
    print(f"  ~{hz:7.0f} Hz  in {n}/22 tracks")

# ---------- 3. similarity of tonal balance
M = np.array([[t["band_balance_db"][b] for b in
               ["sub", "bass", "low_bass", "low_mid", "mid", "up_mid", "presence", "brilliance", "air"]] for t in TRACKS])
Mn = (M - M.mean(axis=0)) / (M.std(axis=0) + 1e-9)
D = np.sqrt(((Mn[:, None, :] - Mn[None, :, :]) ** 2).mean(axis=2))
names = [t["file"].replace(".mp3", "") for t in TRACKS]
order = np.argsort(D.mean(axis=1))
print("\n== tonal-balance distance to the group (lower = more 'typical' for him) ==")
for i in order[:6]:
    print(f"  CLOSEST  {D[i].mean():.2f}  {names[i]}")
print("  ...")
for i in order[-4:]:
    print(f"  OUTLIER  {D[i].mean():.2f}  {names[i]}")

fig, ax = plt.subplots(figsize=(11, 9))
im = ax.imshow(D, cmap="viridis")
ax.set_xticks(range(len(names))); ax.set_xticklabels([n[:18] for n in names], rotation=90, fontsize=6)
ax.set_yticks(range(len(names))); ax.set_yticklabels([n[:18] for n in names], fontsize=6)
ax.set_title("Tonal-balance similarity between tracks")
fig.colorbar(im, ax=ax, label="RMS distance (z-scores)")
fig.tight_layout(); fig.savefig(os.path.join(FIG, "similarity.png"), dpi=130); plt.close(fig)

# ---------- 4. signature EQ curve plot
fig, ax = plt.subplots(figsize=(10, 5.5))
x = np.arange(len(OCT)) + 0.5
ax.bar(x, rel, width=0.75, color=["#8b1a1a" if v > 2 else ("#3b6ea5" if v < -2 else "#888") for v in rel])
ax.axhline(0, color="k", lw=0.8)
ax.set_xticks(x); ax.set_xticklabels(LABELS, rotation=45, ha="right")
ax.set_ylabel("dB re 640 Hz–1.2 kHz band")
ax.set_title("Milad Hashemi — median tonal signature (22 tracks, octave bands)")
ax.grid(axis="y", alpha=0.3)
fig.tight_layout(); fig.savefig(os.path.join(FIG, "signature_curve.png"), dpi=140); plt.close(fig)

# ---------- 5. per-track table rows for the report
rows = []
for t in TRACKS:
    rows.append(dict(
        name=t["file"].replace(".mp3", ""),
        lufs=t["ebur128"].get("lufs_i"), lra=t["ebur128"].get("lra"), tp=t["ebur128"].get("true_peak"),
        corr=t["stereo"].get("lr_correlation"), ms=t["stereo"].get("ms_ratio_db"),
        tilt=t["tilt_db_per_oct_150_8k"], perc=t["hpss"].get("percussive_pct"),
        bpm=t.get("tempo_bpm"), key=t.get("key_estimate"),
    ))
json.dump({"octave_curve_db": dict(zip(LABELS, [round(float(v - med[k_idx]), 2) for v in med])),
           "recurring_peaks": [[round(float(h), 1), n] for h, n in sorted(hot)],
           "closest": [names[i] for i in order[:6]],
           "outliers": [names[i] for i in order[-4:]],
           "per_track": rows}, open("analysis/signature2.json", "w"), indent=1)
print("\nwrote analysis/signature2.json")
