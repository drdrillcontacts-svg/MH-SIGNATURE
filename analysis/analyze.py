#!/usr/bin/env python3
"""
MH-SIGNATURE — mix analysis engine
----------------------------------
Per-track objective measurements used to infer Milad Hashemi's mix signature:
  * loudness / dynamics (LUFS, true peak, LRA, crest, limiter density)
  * spectral colour (long-term PSD, band balance, spectral tilt, centroid,
    rolloff, flatness, resonance peaks, HF cutoff)
  * stereo image (M/S ratio overall + per band, correlation, mono-bass check)
  * space / reverb (per-band energy-decay T60 estimate, tail level)
  * sidechain / pumping (kick-ducking depth, tempo-synced envelope modulation)
  * texture (HPSS harmonic/percussive balance, per-band crest factor)
"""
import os, sys, json, subprocess, re, glob
import numpy as np
import soundfile as sf
from scipy import signal

import pyloudnorm as pyln
import librosa

FFMPEG = os.environ.get("FFMPEG", "ffmpeg")

BANDS = [
    ("sub",        20,    45),
    ("bass",       45,    90),
    ("low_bass",   90,   140),
    ("low_mid",   140,   300),
    ("mid",       300,  1000),
    ("up_mid",   1000,  2500),
    ("presence", 2500,  5000),
    ("brilliance", 5000, 9000),
    ("air",      9000, 14000),
    ("top",     14000, 20000),
]


# ----------------------------------------------------------------------------- io
def load(path):
    x, sr = sf.read(path, always_2d=True, dtype="float32")
    if x.shape[1] == 1:
        x = np.repeat(x, 2, axis=1)
    return x, sr


def db(x, floor=-160.0):
    return np.maximum(20.0 * np.log10(np.maximum(np.abs(x), 1e-12)), floor)


def rms_db(x):
    return float(20 * np.log10(max(np.sqrt(np.mean(np.square(x, dtype=np.float64))), 1e-12)))


def ffmpeg_ebur128(path):
    """Integrated LUFS, LRA and true peak straight from ffmpeg's EBU R128 meter."""
    cmd = [FFMPEG, "-hide_banner", "-nostats", "-i", path,
           "-filter_complex", "ebur128=peak=true", "-f", "null", "-"]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        out = p.stderr
    except Exception:
        return {}
    summary = out.split("Summary:")[-1]
    res = {}
    m = re.search(r"I:\s+(-?[\d.]+) LUFS", summary)
    if m: res["lufs_i"] = float(m.group(1))
    m = re.search(r"LRA:\s+(-?[\d.]+) LU", summary)
    if m: res["lra"] = float(m.group(1))
    m = re.search(r"Peak:\s+(-?[\d.]+) dBFS", summary)
    if m: res["true_peak"] = float(m.group(1))
    if "lufs_i" not in res:
        # true peak may be in True peak: line
        m = re.search(r"True peak:\s+Peak:\s+(-?[\d.]+)", summary)
        if m: res["true_peak"] = float(m.group(1))
    return res


# ------------------------------------------------------------------- spectral
def psd(x, sr, nperseg=8192):
    f, P = signal.welch(x, sr, nperseg=min(nperseg, len(x)), noverlap=min(nperseg, len(x)) // 2,
                        window="hann", scaling="density", detrend=False)
    return f, P


def band_levels(f, P):
    """Band energies in dB relative to total energy."""
    tot = np.trapezoid(P, f)
    out = {}
    for name, lo, hi in BANDS:
        m = (f >= lo) & (f < hi)
        if not m.any():
            out[name] = None
            continue
        e = np.trapezoid(P[m], f[m])
        out[name] = float(10 * np.log10(max(e, 1e-20) / max(tot, 1e-20)))
    return out


def smooth_db(f, P, frac=12):
    """1/frac-octave smoothing of a PSD in dB."""
    fdb = 10 * np.log10(np.maximum(P, 1e-20))
    out = np.copy(fdb)
    lf = np.log2(np.maximum(f, 1.0))
    w = 1.0 / frac
    for i in range(len(f)):
        m = np.abs(lf - lf[i]) <= w / 2
        if m.any():
            out[i] = fdb[m].mean()
    return out


def tilt(f, P, lo=150, hi=8000):
    """Spectral tilt in dB/octave (least squares on log2 frequency)."""
    m = (f >= lo) & (f <= hi) & (P > 0)
    if m.sum() < 8:
        return None
    lf = np.log2(f[m])
    ldb = 10 * np.log10(P[m])
    a, b = np.polyfit(lf, ldb, 1)
    return float(a)


def hf_cutoff(f, Pdb, start=3000, drop=45.0):
    """Highest frequency still carrying meaningful content (detects a lowpass shelf)."""
    m = f >= start
    ff, ll = f[m], Pdb[m]
    if len(ff) == 0:
        return None
    ref = np.percentile(ll, 90)
    below = ll < (ref - drop)
    idx = None
    for i in range(len(ll)):
        if below[i:].all():
            idx = i
            break
    if idx is None:
        return None
    return float(ff[idx])


def find_resonances(f, Pdb, lo=150, hi=12000, rise=5.0, top=8):
    """Peaks that stand above their local neighbourhood."""
    m = (f >= lo) & (f <= hi)
    ff, ll = f[m], Pdb[m]
    lf = np.log2(ff)
    span = 0.33  # +-1/3 octave baseline
    base = np.empty_like(ll)
    for i in range(len(ff)):
        mm = np.abs(lf - lf[i]) <= span
        base[i] = np.median(ll[mm])
    excess = ll - base
    peaks, props = signal.find_peaks(excess, height=rise, distance=8)
    order = np.argsort(props["peak_heights"])[::-1][:top]
    return [{"f": float(ff[p]), "excess_db": float(props["peak_heights"][p])} for p in order]


# ---------------------------------------------------------------------- stereo
def stereo_metrics(L, R, sr):
    mid = (L + R) / 2.0
    side = (L - R) / 2.0
    rms_m, rms_s = np.sqrt(np.mean(mid**2)), np.sqrt(np.mean(side**2))
    corr = float(np.corrcoef(L, R)[0, 1]) if L.std() > 0 and R.std() > 0 else None
    return mid, side, {
        "ms_ratio_db": float(20 * np.log10(max(rms_s, 1e-12) / max(rms_m, 1e-12))),
        "lr_correlation": corr,
    }


def band_width(f, Pm, Ps):
    out = {}
    for name, lo, hi in BANDS:
        m = (f >= lo) & (f < hi)
        if not m.any():
            out[name] = None
            continue
        em, es = np.trapezoid(Pm[m], f[m]), np.trapezoid(Ps[m], f[m])
        out[name] = float(10 * np.log10(max(es, 1e-20) / max(em, 1e-20)))
    return out


def band_crest(x, sr, lo, hi):
    sos = signal.butter(4, [lo, min(hi, sr / 2 - 100)], btype="band", fs=sr, output="sos")
    y = signal.sosfilt(sos, x)
    if y.std() == 0:
        return None
    return float(20 * np.log10(max(np.abs(y).max(), 1e-12)) - rms_db(y))


# -------------------------------------------------------------------- dynamics
def rms_envelope(x, sr, hop_ms=10):
    hop = max(1, int(sr * hop_ms / 1000))
    n = len(x) // hop
    seg = x[:n * hop].reshape(n, hop)
    return np.sqrt(np.mean(seg.astype(np.float64)**2, axis=1)), hop / sr


def short_term_rms(x, sr, win=3.0, hop=0.25):
    env, dt = rms_envelope(x, sr, hop_ms=100)
    w = int(win / dt)
    h = max(1, int(hop / dt))
    vals = [rms_db(env[i:i + w]) for i in range(0, max(1, len(env) - w), h)]
    return np.array(vals) if vals else np.array([rms_db(env)])


def limiter_density(x):
    a = np.abs(x).max(axis=1)
    n = len(a)
    return {
        "pct_above_-0.1": float(100 * np.mean(a >= 10**(-0.1 / 20))),
        "pct_above_-0.5": float(100 * np.mean(a >= 10**(-0.5 / 20))),
        "pct_above_-3": float(100 * np.mean(a >= 10**(-3 / 20))),
        "sample_peak_db": float(db(a.max())),
    }


def clipping_stats(x, thresh_db=-0.3):
    a = np.abs(x)
    ceil = 10 ** (thresh_db / 20)
    hit = (a.max(axis=1) >= ceil).astype(np.int8)
    flat = 0
    run = 0
    for v in hit:
        if v:
            run += 1
        else:
            if run >= 3:
                flat += run
            run = 0
    total = len(hit)
    return {
        "pct_samples_at_ceiling": float(100 * hit.mean()),
        "pct_in_flat_top_runs": float(100 * flat / max(total, 1)),
        "ceiling_db": thresh_db,
    }


def decay_after_peaks(x, sr, lo, hi, hold=0.30, prom=6.0):
    """dB fallen `hold` seconds after band-envelope peaks -> T60 estimate."""
    hi = min(hi, sr / 2 - 200)
    if lo >= hi:
        return None
    sos = signal.butter(4, [lo, hi], btype="band", fs=sr, output="sos")
    y = signal.sosfilt(sos, x)
    hop = max(1, int(sr * 0.010))
    env = np.abs(y)
    win = max(2, int(sr * 0.020))
    env = np.convolve(env, np.ones(win) / win, mode="same")[::hop]
    e = 20 * np.log10(np.maximum(env, 1e-12))
    dt = hop / sr
    pk, _ = signal.find_peaks(e, distance=int(0.20 / dt), prominence=prom)
    h = int(hold / dt)
    drops = []
    for p in pk:
        if p + h >= len(e):
            continue
        drops.append(e[p] - e[p + h])
    drops = [d for d in drops if d > 1.5]
    if len(drops) < 8:
        return None
    d = float(np.median(drops))
    return {"drop_db_in_%dms" % int(hold * 1000): round(d, 2),
            "t60_est_s": round(60.0 * hold / d, 3) if d > 0 else None}


def decay_time(x, sr, lo, hi, min_drop=18.0, min_len=0.25):
    """Rough per-band T60 from monotonic decays of the amplitude envelope (dB/s -> 60dB)."""
    hi = min(hi, sr / 2 - 200)
    if lo >= hi:
        return None
    sos = signal.butter(4, [lo, hi], btype="band", fs=sr, output="sos")
    y = signal.sosfilt(sos, x)
    env = np.abs(y)
    win = max(2, int(sr * 0.020))
    env = np.convolve(env, np.ones(win) / win, mode="same")
    hop = max(1, int(sr * 0.010))
    e = 20 * np.log10(np.maximum(env[::hop], 1e-12))
    if len(e) < 100:
        return None
    t = np.arange(len(e)) * hop / sr
    # find local maxima then follow the decay
    pk, _ = signal.find_peaks(e, distance=int(0.08 / (hop / sr)), prominence=6)
    slopes = []
    step = int(0.020 / (hop / sr))
    for p in pk:
        seg = [e[p]]
        i = p
        while i + step < len(e):
            i += step
            if e[i] <= seg[-1] + 0.3:
                seg.append(e[i])
            else:
                break
        if len(seg) * 0.020 >= min_len and (seg[0] - seg[-1]) >= min_drop:
            tt = np.arange(len(seg)) * 0.020
            a = np.polyfit(tt, np.array(seg), 1)[0]
            if a < -3:
                slopes.append(-a)
    if not slopes:
        return None
    return float(60.0 / np.median(slopes))


def pumping(x_mono, sr):
    """Look for kick-triggered ducking of the rest of the mix."""
    env_full, dt = rms_envelope(x_mono, sr, hop_ms=10)
    sos = signal.butter(4, [35, 120], btype="band", fs=sr, output="sos")
    low = signal.sosfilt(sos, x_mono)
    env_low, _ = rms_envelope(low, sr, hop_ms=10)
    n = min(len(env_full), len(env_low))
    if n < 100:
        return {}
    ef, el = 20 * np.log10(np.maximum(env_full[:n], 1e-12)), 20 * np.log10(np.maximum(env_low[:n], 1e-12))
    pk, _ = signal.find_peaks(el, distance=int(0.25 / dt), prominence=8)
    if len(pk) < 20:
        return {"kick_events": int(len(pk))}
    full_med = np.median(ef)
    durs = []
    a, b = int(0.05 / dt), int(0.15 / dt)
    for p in pk:
        if p + b < n:
            durs.append(np.median(ef[p + a:p + b]) - full_med)
    # envelope modulation periodicity
    ac = None
    seg = ef - ef.mean()
    c = np.correlate(seg, seg, mode="full")[len(seg) - 1:]
    if c[0] > 0:
        c = c / c[0]
        lo_l = int(60 / 190 / dt)   # lag for 190 BPM (fastest)
        hi_l = int(60 / 60 / dt)    # lag for 60 BPM (slowest)
        hi_l = min(hi_l, len(c) - 1)
        if hi_l > lo_l + 2:
            k = lo_l + int(np.argmax(c[lo_l:min(hi_l, len(c) - 1)]))
            ac = float(c[k])
            period = k * dt
        else:
            period = None
    else:
        period = None
    return {
        "kick_events": int(len(pk)),
        "duck_db_at_50_150ms": float(np.median(durs)) if durs else None,
        "env_mod_peak": ac,
        "env_mod_bpm": float(60 / period) if period else None,
    }


def hpss_balance(x_mono, sr):
    try:
        y = librosa.resample(x_mono, orig_sr=sr, target_sr=22050)
        h, p = librosa.effects.hpss(y, margin=1.0)
        eh, ep = float(np.sum(h**2)), float(np.sum(p**2))
        return {"harmonic_pct": float(100 * eh / max(eh + ep, 1e-20)),
                "percussive_pct": float(100 * ep / max(eh + ep, 1e-20)),
                "residual_pct": float(100 * (np.sum(y**2) - eh - ep) / max(np.sum(y**2), 1e-20))}
    except Exception as e:
        return {"error": str(e)}


def loudest_window(x_mono, sr, seconds=60.0):
    env, dt = rms_envelope(x_mono, sr, hop_ms=500)
    w = max(1, int(seconds / dt))
    if len(env) <= w:
        return x_mono, 0.0
    c = np.convolve(env, np.ones(w) / w, mode="valid")
    i = int(np.argmax(c))
    return x_mono[int(i * dt * sr): int(i * dt * sr) + int(seconds * sr)], i * dt


# ------------------------------------------------------------------------ main
def analyse(path):
    x, sr = load(path)
    L, R = x[:, 0], x[:, 1]
    mono = (L + R) / 2.0
    dur = len(mono) / sr
    res = {"file": os.path.basename(path), "sample_rate": sr, "channels": 2,
           "duration_s": round(dur, 2)}

    # ---- loudness / dynamics
    res["ebur128"] = ffmpeg_ebur128(path)
    try:
        meter = pyln.Meter(sr)
        res["lufs_i_pyloudnorm"] = float(meter.integrated_loudness(x.astype(np.float64)))
    except Exception:
        pass
    st = short_term_rms(mono, sr)
    st_rel = st[st > (np.max(st) - 45.0)]   # ignore digital silence / fades
    if st_rel.size < 4:
        st_rel = st
    res["dynamics"] = {
        "short_term_p10": float(np.percentile(st, 10)),
        "short_term_p50": float(np.percentile(st, 50)),
        "short_term_p90": float(np.percentile(st, 90)),
        "short_term_range": float(np.percentile(st, 90) - np.percentile(st, 10)),
        "short_term_std": float(np.std(st_rel)),
        "short_term_p05": float(np.percentile(st_rel, 5)),
        "short_term_p95": float(np.percentile(st_rel, 95)),
        "rms_db": rms_db(mono),
        "crest_db": float(db(np.abs(x).max()) - rms_db(mono)),
    }
    res["limiter"] = limiter_density(x)
    tp = res["ebur128"].get("true_peak")
    lufs = res["ebur128"].get("lufs_i")
    if tp is not None and lufs is not None:
        res["dynamics"]["plr"] = float(tp - lufs)

    # ---- spectral
    f, Pm = psd(mono, sr)
    _, Ps = psd((L - R) / 2.0, sr)
    Pdb = 10 * np.log10(np.maximum(Pm, 1e-20))
    res["band_balance_db"] = band_levels(f, Pm)
    res["tilt_db_per_oct_150_8k"] = tilt(f, Pm)
    res["tilt_db_per_oct_1k_10k"] = tilt(f, Pm, 1000, 10000)
    res["tilt_db_per_oct_4k_16k"] = tilt(f, Pm, 4000, 16000)
    sm = smooth_db(f, Pm)
    tot = Pdb.max()
    res["spectral"] = {
        "centroid_hz": float(np.sum(f * Pm) / max(np.sum(Pm), 1e-20)),
        "rolloff85_hz": float(f[np.searchsorted(np.cumsum(Pm) / np.sum(Pm), 0.85)]),
        "rolloff95_hz": float(f[np.searchsorted(np.cumsum(Pm) / np.sum(Pm), 0.95)]),
        "hf_cutoff_hz": hf_cutoff(f, sm),
        "resonances": find_resonances(f, sm),
        "level_at_1k_db": float(np.interp(1000, f, sm)),
        "spectrum_db": [round(float(v - tot), 2) for v in sm[::2]],
        "spectrum_freqs": [round(float(v), 1) for v in f[::2]],
    }

    # ---- stereo
    mid, side, sm_stats = stereo_metrics(L, R, sr)
    res["stereo"] = sm_stats
    res["stereo"]["band_ms_db"] = band_width(f, Pm, Ps)

    # ---- band crest (texture / compression per band)
    res["band_crest_db"] = {}
    for name, lo, hi in BANDS:
        try:
            res["band_crest_db"][name] = band_crest(mono, sr, lo, hi)
        except Exception:
            res["band_crest_db"][name] = None

    # ---- heavy analyses on the loudest 60 s
    win, off = loudest_window(mono, sr, 60.0)
    res["analysis_window_s"] = [round(off, 1), round(off + len(win) / sr, 1)]
    res["pumping"] = pumping(win, sr)
    res["hpss"] = hpss_balance(win, sr)
    res["decay"] = {
        "broadband": decay_after_peaks(win, sr, 60, 16000),
        "low_60_250": decay_after_peaks(win, sr, 60, 250),
        "mid_250_2k": decay_after_peaks(win, sr, 250, 2000),
        "high_2k_8k": decay_after_peaks(win, sr, 2000, 8000),
        "air_8k_14k": decay_after_peaks(win, sr, 8000, 14000, prom=8.0),
    }

    res["clipping"] = clipping_stats(x)

    # ---- derived tonal ratios (relative, easy to read)
    bb = res["band_balance_db"]
    def _r(a, b):
        return round(bb[a] - bb[b], 1) if bb.get(a) is not None and bb.get(b) is not None else None
    res["tonal_ratios_db"] = {
        "bass_vs_mid": _r("bass", "mid"),
        "sub_vs_bass": _r("sub", "bass"),
        "lowmid_vs_mid": _r("low_mid", "mid"),
        "mid_vs_upmid": _r("mid", "up_mid"),
        "presence_vs_mid": _r("presence", "mid"),
        "brilliance_vs_presence": _r("brilliance", "presence"),
        "air_vs_brilliance": _r("air", "brilliance"),
        "harsh_2_5k_vs_1k": round(float(np.interp(3500, f, sm) - np.interp(1000, f, sm)), 1),
        "sib_6_9k_vs_3_5k": round(float(np.mean(sm[(f >= 6000) & (f <= 9000)]) - np.mean(sm[(f >= 3000) & (f <= 5000)])), 1),
        "mud_250_400_vs_800": round(float(np.mean(sm[(f >= 250) & (f <= 400)]) - np.interp(800, f, sm)), 1),
    }

    # ---- tempo / key
    try:
        y22 = librosa.resample(mono, orig_sr=sr, target_sr=22050)
        tempo, beats = librosa.beat.beat_track(y=y22, sr=22050)
        res["tempo_bpm"] = float(np.atleast_1d(tempo)[0])
        chroma = librosa.feature.chroma_cqt(y=y22, sr=22050).mean(axis=1)
        names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
        res["key_estimate"] = names[int(np.argmax(chroma))]
    except Exception as e:
        res["tempo_bpm"] = None

    return res


if __name__ == "__main__":
    files = sys.argv[1:] or sorted(glob.glob("google_drive/*.mp3"))
    outdir = "analysis/out"
    os.makedirs(outdir, exist_ok=True)
    for fp in files:
        slug = re.sub(r"[^A-Za-z0-9]+", "_", os.path.basename(fp).rsplit(".", 1)[0]).strip("_")
        dst = os.path.join(outdir, slug + ".json")
        if os.path.exists(dst):
            print("skip", slug)
            continue
        try:
            r = analyse(fp)
            r["slug"] = slug
            with open(dst, "w") as fh:
                json.dump(r, fh)
            print(f"OK {slug}  LUFS={r.get('ebur128',{}).get('lufs_i')}  TP={r.get('ebur128',{}).get('true_peak')}")
        except Exception as e:
            import traceback; traceback.print_exc()
            print("FAIL", fp, e)
