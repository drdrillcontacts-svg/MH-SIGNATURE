#!/usr/bin/env python3
"""Generate Source/Presets.cpp — 100 factory presets for MH SIGNATURE.

Everything derives from the measured medians of Milad Hashemi's 22 mixes:
  LUFS -7.6 · PLR 9.3 · tilt -5.37 dB/oct · T60 mid 3.4 s/low 1.79 s
  side/mid -7.67 dB · correlation 0.71 · presence dip -12.8 dB
"""
import os, random

random.seed(20261028)

GROUPS = [
    ("Signature", 10, dict(char=(60, 76), dens=(54, 70), space=(58, 78), width=(30, 46),
                           extra=dict(sig_intensity=(90, 100), lim_ceiling=(-0.6, -0.2)))),
    ("Ballad", 10, dict(char=(52, 68), dens=(42, 58), space=(76, 92), width=(24, 40),
                        extra=dict(rev_decay=(4.0, 6.5), rev_mix=(28, 40), dly_mix=(12, 24),
                                   rev_damp=(68, 82), lim_ceiling=(-1.2, -0.4)))),
    ("Pop", 10, dict(char=(58, 72), dens=(64, 80), space=(52, 70), width=(42, 62),
                     extra=dict(lim_clip=(1.5, 3.0), dly_mix=(14, 26), mod_mix=(12, 24)))),
    ("Rock", 10, dict(char=(52, 68), dens=(68, 84), space=(44, 62), width=(34, 56),
                      extra=dict(sat_drive=(6.0, 12.0), eq_hm_g=(-4.5, -1.5),
                                 lim_clip=(2.0, 4.0)))),
    ("Hip-Hop", 10, dict(char=(62, 78), dens=(72, 90), space=(40, 58), width=(26, 44),
                         extra=dict(eq_low_g=(3.5, 6.5), eq_lm_g=(-4.0, -1.0),
                                    lim_clip=(2.0, 4.5), dly_mix=(8, 18)))),
    ("Electronic", 10, dict(char=(56, 72), dens=(60, 78), space=(62, 86), width=(52, 78),
                            extra=dict(mod_mix=(22, 40), mod_depth=(30, 55),
                                       dly_mix=(20, 34), img_w_air=(120, 150)))),
    ("Acoustic", 10, dict(char=(42, 58), dens=(34, 52), space=(58, 74), width=(30, 48),
                          extra=dict(eq_high_g=(-3.5, -0.5), voc_air_g=(-4.5, -1.0),
                                     sat_mix=(30, 50), rev_mix=(20, 32)))),
    ("Vocal Focus", 10, dict(char=(58, 72), dens=(56, 72), space=(56, 74), width=(34, 50),
                             extra=dict(voc_presence_g=(-2.0, 1.5), voc_deess_range=(-6.5, -3.0),
                                        voc_body_g=(0.5, 3.5), sat_drive=(2.5, 6.0)))),
    ("Dark Room", 10, dict(char=(78, 94), dens=(56, 72), space=(72, 90), width=(20, 38),
                           extra=dict(eq_high_g=(-9.0, -5.0), voc_air_g=(-12.0, -7.0),
                                      rev_damp=(76, 90), sat_hf=(6500, 8500)))),
    ("Streaming", 10, dict(char=(58, 72), dens=(50, 66), space=(54, 72), width=(34, 52),
                           extra=dict(lim_ceiling=(-1.4, -1.0), lim_clip=(0.0, 1.0),
                                      io_output=(-2.5, -1.0), rev_mix=(16, 26)))),
]

# (friendly name written under the category header)
NAMES = {
    "Signature":   ["Modern Clarity", "Warm Body", "Wide Air", "Deep Core", "Silk Top",
                    "Neutral Base", "Studio Standard", "Analog Warmth", "Soft Focus", "Balanced Print"],
    "Ballad":      ["Slow Bloom", "Night Piano", "Velvet Space", "Long Tail", "Quiet Fire",
                    "String Wash", "Soft Ember", "Dim Lantern", "Open Window", "Late Letter"],
    "Pop":         ["Radio Bright", "Punch Forward", "Clean Lift", "Tight Low", "Fast Hook",
                    "Chorus Lift", "Dance Floor", "Modern Gloss", "Snap Attack", "Big Chorus"],
    "Rock":        ["Guitar Wall", "Bass Grit", "Drum Room", "Tube Drive", "Live Stage",
                    "Heavy Body", "Bright Edge", "Raw Power", "Classic Stack", "Wide Drums"],
    "Hip-Hop":     ["Sub Anchor", "808 Weight", "Tight Kick", "Dark Trap", "Vocal Pocket",
                    "Boom Bap", "Modern Trap", "Low Rider", "Clean Sub", "Street Print"],
    "Electronic":  ["Neon Pulse", "Modulated Air", "Wide Synth", "Deep Sequence", "Arp Space",
                    "Club Wide", "Ambient Wash", "Digital Sheen", "Pulse Width", "Night Drive"],
    "Acoustic":    ["Wood Body", "String Air", "Natural Room", "Finger Style", "Soft Pick",
                    "Open Guitar", "Live Hall", "Clean Pair", "Warm Wood", "Close Mic"],
    "Vocal Focus": ["Lead Forward", "Whisper Detail", "Power Ballad", "De-Essed Air", "Body Warm",
                    "Speech Clarity", "Doubled Lead", "Presence Lift", "Soft Consonant", "Rap Vocal"],
    "Dark Room":   ["Obsidian", "Smoke Room", "Black Tape", "Low Ceiling", "Ember Glow",
                    "Muted Top", "Deep Shadow", "Dark Plate", "Ash Room", "Midnight"],
    "Streaming":   ["Safe Master", "Spotify Ready", "Apple Ready", "YouTube Ready", "True Peak Safe",
                    "Dynamics Kept", "Loudness Trim", "Headroom Plus", "Club Safe", "Broadcast"],
}


def rng(lo_hi):
    lo, hi = lo_hi
    return round(random.uniform(lo, hi), 2)


def build():
    out = []
    idx = 0
    for group, count, spec in GROUPS:
        for i in range(count):
            idx += 1
            base = dict(
                sig_character=float(rng(spec["char"])),
                sig_density=float(rng(spec["dens"])),
                sig_space=float(rng(spec["space"])),
                sig_width=float(rng(spec["width"])),
            )
            vals = dict(base)
            vals.update({k: float(rng(v)) for k, v in spec.get("extra", {}).items()})

            # small deterministic jitter on shared tone-shaping parameters
            vals.setdefault("eq_low_g", round(base["sig_character"] * 0.045, 2))
            vals.setdefault("eq_hm_g", round(-2.0 - base["sig_density"] * 0.03, 2))
            vals.setdefault("eq_high_g", round(-1.0 - base["sig_character"] * 0.05, 2))
            vals.setdefault("rev_mix", round(8.0 + base["sig_space"] * 0.22, 2))
            vals.setdefault("img_width", round(80.0 + base["sig_width"] * 1.4, 2))
            vals.setdefault("dyn_thr", round(-26.0 + base["sig_density"] * 0.10, 2))
            vals.setdefault("sat_drive", round(2.0 + base["sig_character"] * 0.04, 2))

            name = NAMES[group][i]
            out.append((f"{idx:03d} {name}", group, vals))
    return out


def main():
    presets = build()
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    hdr = """/*  Auto-generated by tools/gen_presets.py — do not edit by hand.  */
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

struct FactoryPreset
{
    juce::String name;
    juce::String category;
    std::vector<std::pair<juce::String, float>> values;
};

const std::vector<FactoryPreset>& getFactoryPresets();
"""
    open(os.path.join(root, "Source", "Presets.h"), "w").write(hdr)

    lines = []
    lines.append("/*  Auto-generated by tools/gen_presets.py — do not edit by hand.  */")
    lines.append('#include "Presets.h"')
    lines.append("")
    lines.append("namespace")
    {8}
    lines.append("{")
    lines.append("struct Entry { const char* name; const char* cat; "
                 "const char* const* ids; const float* vals; int count; };")
    lines.append("")

    table_entries = []
    for n, (name, cat, vals) in enumerate(presets):
        ids = ", ".join(f'"{k}"' for k in vals.keys())
        flt = ", ".join(f"{v}f" for v in vals.values())
        lines.append(f"const char* const ids{n}[] = {{ {ids} }};")
        lines.append(f"const float vals{n}[] = {{ {flt} }};")
        table_entries.append(f'    {{ "{name}", "{cat}", ids{n}, vals{n}, {len(vals)} }},')

    lines.append("")
    lines.append("const Entry entries[] = {")
    lines.extend(table_entries)
    lines.append("};")
    lines.append("")
    lines.append("constexpr int numEntries = (int) (sizeof (entries) / sizeof (Entry));")
    lines.append("")
    lines.append("}")
    lines.append("")
    lines.append("const std::vector<FactoryPreset>& getFactoryPresets()")
    lines.append("{")
    lines.append("    static const std::vector<FactoryPreset> presets = []")
    lines.append("    {")
    lines.append("        std::vector<FactoryPreset> v;")
    lines.append("        v.reserve ((size_t) numEntries);")
    lines.append("        for (int i = 0; i < numEntries; ++i)")
    lines.append("        {")
    lines.append("            FactoryPreset p;")
    lines.append("            p.name = juce::String (entries[i].name);")
    lines.append("            p.category = juce::String (entries[i].cat);")
    lines.append("            for (int k = 0; k < entries[i].count; ++k)")
    lines.append("                p.values.emplace_back (juce::String (entries[i].ids[k]),")
    lines.append("                                      entries[i].vals[k]);")
    lines.append("            v.push_back (std::move (p));")
    lines.append("        }")
    lines.append("        return v;")
    lines.append("    }();")
    lines.append("    return presets;")
    lines.append("}")
    lines.append("")

    body = "\n".join(lines).replace("{8}", "{")
    open(os.path.join(root, "Source", "Presets.cpp"), "w").write(body)

    cats = {}
    for _, c, _ in presets:
        cats[c] = cats.get(c, 0) + 1
    print(f"generated {len(presets)} presets across {len(cats)} categories")
    for c, n in cats.items():
        print(f"  {c:14s} {n}")


if __name__ == "__main__":
    main()
