#!/usr/bin/env python3
"""Aggregate per-track analyses into a signature summary + figures."""
import os, json, glob
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

OUT = "analysis/out"
FIG = "analysis/figs"
os.makedirs(FIG, exist_ok=True)

files = sorted(glob.glob(os.path.join(OUT, "*.json")))
rows = []
for fp in files:
    d = json.load(open(fp))
    eb, dy, lm, cl = d.get("ebur128", {}), d.get("dynamics", {}), d.get("limiter", {}), d.get("clipping", {})
    bb, tr, st = d.get("band_balance_db", {}), d.get("tonal_ratios_db", {}), d.get("stereo", {})
    bw, bc, dec = st.get("band_ms_db", {}), d.get("band_crest_db", {}), d.get("decay", {})
    rows.append({
        "slug": d["slug"],
        "name": d["file"].replace(".mp3", ""),
        "dur": d["duration_s"],
        "bpm": d.get("tempo_bpm"),
        "key": d.get("key_estimate"),
        "lufs": eb.get("lufs_i"),
        "lra": eb.get("lra"),
        "tp": eb.get("true_peak"),
        "crest": dy.get("crest_db"),
        "plr": dy.get("plr"),
        "st_range": dy.get("short_term_range"),
        "st_std": dy.get("short_term_std"),
        "tb_pct": lm.get("pct_above_-0.1"),
        "clip_pct": cl.get("pct_samples_at_ceiling"),
        "flat_pct": cl.get("pct_in_flat_top_runs"),
        "tilt": d.get("tilt_db_per_oct_150_8k"),
        "tilt_hf": d.get("tilt_db_per_oct_4k_16k"),
        "centroid": d.get("spectral", {}).get("centroid_hz"),
        "rolloff95": d.get("spectral", {}).get("rolloff95_hz"),
        "hf_cut": d.get("spectral", {}).get("hf_cutoff_hz"),
        "sub": bb.get("sub"), "bass": bb.get("bass"), "low_bass": bb.get("low_bass"),
        "low_mid": bb.get("low_mid"), "mid": bb.get("mid"), "up_mid": bb.get("up_mid"),
        "presence": bb.get("presence"), "brill": bb.get("brilliance"),
        "air": bb.get("air"), "top": bb.get("top"),
        "bass_vs_mid": tr.get("bass_vs_mid"),
        "pres_vs_mid": tr.get("presence_vs_mid"),
        "air_vs_brill": tr.get("air_vs_brilliance"),
        "harsh": tr.get("harsh_2_5k_vs_1k"),
        "sib": tr.get("sib_6_9k_vs_3_5k"),
        "ms": st.get("ms_ratio_db"),
        "corr": st.get("lr_correlation"),
        "w_sub": bw.get("sub"), "w_bass": bw.get("bass"),
        "w_mid": bw.get("mid"), "w_pres": bw.get("presence"), "w_air": bw.get("air"),
        "crest_mid": bc.get("mid"), "crest_pres": bc.get("presence"),
        "crest_air": bc.get("air"), "crest_bass": bc.get("bass"),
        "t60_bb": (dec.get("broadband") or {}).get("t60_est_s"),
        "t60_low": (dec.get("low_60_250") or {}).get("t60_est_s"),
        "t60_mid": (dec.get("mid_250_2k") or {}).get("t60_est_s"),
        "t60_high": (dec.get("high_2k_8k") or {}).get("t60_est_s"),
        "harm_pct": d.get("hpss", {}).get("harmonic_pct"),
        "perc_pct": d.get("hpss", {}).get("percussive_pct"),
        "duck": d.get("pumping", {}).get("duck_db_at_50_150ms"),
        "mod_bpm": d.get("pumping", {}).get("env_mod_bpm"),
        "mod_peak": d.get("pumping", {}).get("env_mod_peak"),
    })

# ---------------------------------------------------------------- csv
import csv
keys = list(rows[0].keys())
with open("analysis/summary.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=keys)
    w.writeheader()
    for r in rows:
        w.writerow({k: (round(v, 3) if isinstance(v, float) else v) for k, v in r.items()})

# ---------------------------------------------------------------- stats
def stats(key):
    v = [r[key] for r in rows if isinstance(r[key], (int, float))]
    if not v:
        return None
    return dict(n=len(v), median=float(np.median(v)), mean=float(np.mean(v)),
                sd=float(np.std(v)), lo=float(np.min(v)), hi=float(np.max(v)))

numeric = [k for k in keys if k not in ("slug", "name", "key")]
summary = {k: stats(k) for k in numeric}
json.dump({"per_track": rows, "stats": summary}, open("analysis/signature.json", "w"), indent=1)

# ---------------------------------------------------------------- figures
names = [r["name"] for r in rows]
short = [n[:22] for n in names]

def bar(key, title, unit="", ref=None, fname=None):
    v = [r[key] if isinstance(r[key], (int, float)) else np.nan for r in rows]
    fig, ax = plt.subplots(figsize=(11, 5))
    idx = np.argsort(v)
    ax.barh([short[i] for i in idx], [v[i] for i in idx], color="#3b6ea5")
    ax.set_title(title, fontsize=12)
    ax.set_xlabel(unit)
    if ref is not None:
        ax.axvline(ref, color="crimson", ls="--", lw=1, label=f"ref {ref}")
        ax.legend()
    ax.tick_params(labelsize=7)
    fig.tight_layout()
    fig.savefig(os.path.join(FIG, fname), dpi=130)
    plt.close(fig)

bar("lufs", "Integrated loudness (LUFS)", "LUFS", -14, "lufs.png")
bar("tp", "True peak (dBTP, incl. MP3 overshoot)", "dBTP", 0, "truepeak.png")
bar("crest", "Crest factor (dB)", "dB", None, "crest.png")
bar("ms", "Mid/Side balance (dB, - = narrow)", "dB", None, "width.png")
bar("tilt", "Spectral tilt 150 Hz-8 kHz (dB/oct)", "dB/oct", None, "tilt.png")
bar("perc_pct", "Percussive share of energy (%)", "%", None, "percussive.png")
bar("clip_pct", "Samples at ceiling (%)", "%", None, "clipping.png")

# band heatmap
BANDORDER = ["sub", "bass", "low_bass", "low_mid", "mid", "up_mid", "presence", "brilliance", "air", "top"]
bandnames = ["sub", "bass", "low_bass", "low_mid", "mid", "up_mid", "presence", "brill", "air", "top"]
M = np.array([[r[b] if isinstance(r[b], (int, float)) else np.nan for b in bandnames] for r in rows])
fig, ax = plt.subplots(figsize=(9, 8))
im = ax.imshow(M, aspect="auto", cmap="magma", vmin=np.nanmin(M), vmax=np.nanmax(M))
ax.set_xticks(range(len(bandnames)))
ax.set_xticklabels(bandnames, rotation=45, ha="right", fontsize=8)
ax.set_yticks(range(len(rows)))
ax.set_yticklabels(short, fontsize=6)
ax.set_title("Band balance (dB relative to total energy) — his tonal signature", fontsize=11)
fig.colorbar(im, ax=ax, label="dB")
fig.tight_layout()
fig.savefig(os.path.join(FIG, "band_heatmap.png"), dpi=130)
plt.close(fig)

# average spectrum + spread
fig, ax = plt.subplots(figsize=(11, 6))
allsp = []
for fp in files:
    d = json.load(open(fp))
    sp, fr = d["spectral"]["spectrum_db"], d["spectral"]["spectrum_freqs"]
    allsp.append(sp)
    ax.semilogx(fr, sp, color="grey", alpha=0.25, lw=0.6)
A = np.array(allsp)
med = np.median(A, axis=0)
fr = fr[:A.shape[1]]
ax.semilogx(fr, med, color="crimson", lw=2.2, label="median of 22 tracks")
ax.fill_between(fr, np.percentile(A, 25, axis=0), np.percentile(A, 75, axis=0), color="crimson", alpha=0.18)
ax.set_xlim(20, 20000)
ax.set_ylim(-90, 0)
ax.set_xlabel("Hz")
ax.set_ylabel("dB (re. peak of long-term spectrum)")
ax.set_title("Long-term average spectrum — Milad Hashemi mix signature")
ax.grid(True, which="both", alpha=0.25)
ax.legend()
fig.tight_layout()
fig.savefig(os.path.join(FIG, "avg_spectrum.png"), dpi=140)
plt.close(fig)

# width vs frequency
fig, ax = plt.subplots(figsize=(9, 5))
W = np.array([[json.load(open(fp))["stereo"]["band_ms_db"].get(b) for b in BANDORDER] for fp in files], dtype=float)
ax.boxplot([W[:, i][~np.isnan(W[:, i])] for i in range(W.shape[1])],
           tick_labels=BANDORDER)
ax.set_ylabel("Side/Mid (dB)")
ax.set_title("Stereo width by band (median, IQR across 22 tracks)")
ax.grid(alpha=0.3)
fig.tight_layout()
fig.savefig(os.path.join(FIG, "width_by_band.png"), dpi=140)
plt.close(fig)

print(json.dumps({k: (round(v["median"], 2) if v else None) for k, v in summary.items() if v}, indent=1))
