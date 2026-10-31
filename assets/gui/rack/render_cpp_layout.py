"""Preview of the C++ editor geometry.

PluginEditor.cpp lays every plate out with the same numbers this script uses, so
this is the quickest way to look at a page without opening a DAW.  The art
primitives come from render_v3.py — they are the ones the C++ was ported from.

    python3 assets/gui/rack/render_cpp_layout.py [all|eq|dyn|colour|space|mod|image]

Writes assets/gui/rack/v3-chat-cpp-<page>.jpg
"""

import os
import sys
import math

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import render_v3 as R
from PIL import ImageDraw

INK, INK2, DIM, SOFT = R.INK, R.INK2, R.DIM, R.SOFT

# ------------------------------------------------------------------ dial data
D = {
    "engine":  [("INTENSITY", "100%", 1.00)],
    "eq":      [("HPF", "32 Hz", 0.07), ("LOW", "+3.0 dB", 0.58), ("LO MID", "-2.4 dB", 0.43),
                ("HI MID", "-3.5 dB", 0.40), ("HIGH", "-4.0 dB", 0.39)],
    # the ALL page shows the primary dials only (HPF lives on the focus page)
    "eq_all":  [("LOW", "+3.0 dB", 0.58), ("LO MID", "-2.4 dB", 0.43),
                ("HI MID", "-3.5 dB", 0.40), ("HIGH", "-4.0 dB", 0.39)],
    "dyn":     [("THRESH", "-18.0 dB", 0.70), ("RATIO", "4.0:1", 0.16),
                ("ATTACK", "10 ms", 0.09), ("RELEASE", "100 ms", 0.09),
                ("MAKEUP", "+3.2 dB", 0.13), ("CEILING", "-0.3 dB", 0.96)],
    "colour":  [("DRIVE", "4.5", 0.19), ("TONE", "37%", 0.37), ("MIX", "58%", 0.58)],
    "vocal":   [("HPF", "95 Hz", 0.26), ("BODY", "+2.0 dB", 0.58), ("DE-ESS", "6.5 k", 0.42),
                ("PRES", "-1.5 dB", 0.44), ("AIR", "-6.0 dB", 0.33)],
    "vocal_all": [("BODY", "+2.0 dB", 0.58), ("DE-ESS", "6.5 k", 0.42),
                  ("PRES", "-1.5 dB", 0.44), ("AIR", "-6.0 dB", 0.33)],
    "reverb":  [("DECAY", "3.4 s", 0.27), ("PREDLY", "26 ms", 0.13), ("DAMP", "70%", 0.70),
                ("MIX", "24%", 0.24)],
    "delay":   [("TIME", "375 ms", 0.18), ("FDBK", "36%", 0.38), ("FILTER", "7.2 k", 0.33),
                ("MIX", "18%", 0.18)],
    "motion":  [("RATE", "0.30", 0.05), ("DEPTH", "28%", 0.28), ("MIX", "16%", 0.16),
                ("ATTACK", "+12", 0.56), ("SUSTAIN", "-8", 0.46), ("SPLIT", "180 Hz", 0.07)],
    "trns":    [("ATTACK", "+12", 0.56), ("SUSTAIN", "-8", 0.46), ("SPLIT", "180 Hz", 0.07)],
    "img":     [("WIDTH", "132%", 0.66), ("BAL", "2", 0.52), ("MONO", "92 Hz", 0.26)],
    "imgband": [("LOW", "0%", 0.02), ("MID", "55%", 0.55), ("HIGH", "85%", 0.85),
                ("AIR", "120%", 0.60)],
}

img = R.new_board()


# ------------------------------------------------------------------ one plate
def labelled_dial(cx, cy, r, lab, val, frac):
    """dial() draws its own label/read-out; here the type scale is the C++ one."""
    d = ImageDraw.Draw(img)
    label_size = max(8.0, min(11.0, r * 0.30))
    value_size = max(7.4, min(10.0, r * 0.26))

    R.dial(img, cx, cy, r, "", "", frac)

    d = ImageDraw.Draw(img)
    R.text(d, (cx, cy + r + 15), lab, label_size, INK2, track=1.4)
    w = max(min(62.0 if r >= 40 else 42.0, r * 1.6 + 14.0), len(val) * (value_size * 0.62) + 12.0)
    R.led(img, (cx - w / 2, cy + r + 25, cx + w / 2, cy + r + 43), val, size=value_size)


def panel(box, title, key, rows=1, max_r=26.0, art=None, art_h=0.0, chip=False,
          sections=None, readout="", powers=(("", True),), art_img=None):
    R.plate(img, box, "", True)
    d = ImageDraw.Draw(img)

    x0, y0, x1, y1 = box
    w, h = x1 - x0, y1 - y0
    n = len(powers)

    if title:
        R.text(d, (x0 + 30, y0 + 20), title.upper(), 11.0, INK, track=2.2, anchor="lm")

    px_ = x1 - 30
    for name, on in reversed(powers):
        single = (n == 1)
        px_ -= 84 if single else 44
        if single:
            R.toggle(img, px_, y0 + 10, on, 46, 18, True)
        else:
            R.toggle(img, px_, y0 + 11, on, 30, 16, False)
        px_ -= 0 if single else 8

    if readout:
        right = x1 - 30 - (0 if n == 0 else (90 if n == 1 else 100))
        R.text(d, (right, y0 + 20), readout, 8.2, DIM, track=1.1, anchor="rm")

    R.dline(d, x0 + 26, y0 + 38, x1 - 26, y0 + 38, (10, 11, 13), 1.0)
    R.dline(d, x0 + 26, y0 + 39, x1 - 26, y0 + 39, (44, 48, 55), 0.9)

    top = 48

    if chip:
        cw = min(w - 52, 150)
        d.rounded_rectangle([R.px(x0 + 26), R.px(y0 + top), R.px(x0 + 26 + cw), R.px(y0 + top + 22)],
                            radius=R.px(2.5), fill=R.LED_BG, outline=(44, 48, 55), width=R.px(0.9))
        R.text(d, (x0 + 34, y0 + top + 11), "CHARACTER", 7.0, DIM, track=1.5, anchor="lm")
        R.text(d, (x0 + 26 + cw - 8, y0 + top + 11), "CONSOLE", 7.6, SOFT, track=0.8,
               anchor="rm", mono=True)
        top += 24

    if sections:
        secs = len(sections)
        row_h = (h - 8 - 48) // secs
        per = math.ceil(len(D[key]) / secs)

        for i, lab in enumerate(sections):
            R.text(d, (x0 + 30, y0 + 48 + row_h * i + 8), lab, 8.0, DIM, track=2.2, anchor="lm")
            if i:
                R.dline(d, x0 + 26, y0 + 48 + row_h * i - 2, x1 - 26,
                        y0 + 48 + row_h * i - 2, (12, 13, 16), 1.0)

        for i, (lab, val, frac) in enumerate(D[key]):
            sec, ci = i // per, i % per
            cnt = min(per, len(D[key]) - sec * per)
            slot_w = (w - 32) // cnt
            box_h = row_h - 18
            r = max(9.0, min((slot_w - 30) / 2.0, (box_h - 52) / 2.0, max_r))
            r = round(r)
            cx = x0 + 16 + slot_w * (ci + 0.5)
            cy = y0 + 48 + row_h * sec + 16 + (box_h - (2 * r + 52)) / 2 + r
            labelled_dial(cx, cy, r, lab, val, frac)
        return

    if art_h > 0:
        band = (x0 + 26, y0 + top, x1 - 26, y0 + top + art_h)
        if art_img is not None:
            art_img(img, band)
        else:
            art(ImageDraw.Draw(img), band)
        top += art_h + 12

    box_h = h - 8 - top
    per = math.ceil(len(D[key]) / rows)
    slot_w = (w - 32) // per
    slot_h = box_h // rows
    r = round(max(9.0, min((slot_w - 30) / 2.0, (slot_h - 52) / 2.0, max_r)))

    slack = max(0, slot_h - (2 * r + 52))

    for i, (lab, val, frac) in enumerate(D[key]):
        ri, ci = i // per, i % per
        cx = x0 + 16 + slot_w * (ci + 0.5)
        drop = max(0, slack - 4) if rows == 1 else slack // 2
        cy = y0 + top + slot_h * ri + drop + r
        labelled_dial(cx, cy, r, lab, val, frac)


def engine(box):
    R.plate(img, box, "", True)
    d = ImageDraw.Draw(img)
    x0, y0, x1, y1 = box

    R.text(d, (x0 + 30, y0 + 20), "SIGNATURE ENGINE", 11.6, INK, track=3.0, anchor="lm")
    R.text(d, (x1 - 30, y0 + 20), "TURN IDEAS INTO SOUND", 7.6, DIM, track=2.0, anchor="rm")
    R.dline(d, x0 + 26, y0 + 38, x1 - 26, y0 + 38, (10, 11, 13), 1.0)
    R.dline(d, x0 + 26, y0 + 39, x1 - 26, y0 + 39, (44, 48, 55), 0.9)

    cx = x0 + (x1 - x0) // 2
    labelled_dial(cx, y0 + 74 + 48, 48, "INTENSITY", "100%", 1.0)

    for lab, val, frac, mx, my in (("CHARACTER", "62", .62, 42, 44), ("DENSITY", "48", .48, 42, 146),
                                   ("SPACE", "70", .70, 436, 44), ("WIDTH", "56", .56, 436, 146)):
        labelled_dial(x0 + mx + 34, y0 + my + 19, 19, lab, val, frac)


# ------------------------------------------------------------------ pages
def page_all():
    R.portrait_rail(img)
    R.header(img, "ALL")
    R.footer(img)

    engine((212, 70, 752, 300))
    R.analyser_plate(img, (764, 70, 1132, 300), stats=False)

    panel((212, 312, 510, 582), "EQ", "eq_all", 1, 26.0, R.art_eq, 54, readout="4 BANDS")
    panel((522, 312, 822, 582), "DYNAMICS", "dyn", 2, 26.0, None, 0, readout="GR -6.8 dB",
          powers=(("COMP", True), ("LIMIT", True)))
    panel((834, 312, 1132, 582), "COLOUR", "colour", 1, 26.0, R.art_sat, 74, chip=True)
    panel((212, 594, 510, 848), "VOCAL", "vocal_all", 1, 26.0, R.art_vo, 44,
          readout="DE-ESS 6.5K")
    panel((522, 594, 822, 848), "REVERB", "reverb", 1, 26.0, R.art_decay, 40, readout="3.4 s",
          powers=(("REVERB", True), ("DELAY", True)))
    panel((834, 594, 1132, 848), "MOTION", "motion", 2, 22.0, None, 0,
          sections=("MODULATION", "TRANSIENT"), powers=(("MOD", True), ("TRNS", True)))

    R.chain_column(img, R.RAIL, active=("ENGINE",))


def page_eq():
    R.portrait_rail(img)
    R.header(img, "EQ")
    R.footer(img)
    panel((212, 70, 820, 560), "EQUALISER", "eq", 1, 44.0, R.art_eq, 170, readout="4 BANDS + HPF")
    R.analyser_plate(img, (832, 70, 1132, 560), stats=True)
    panel((212, 572, 662, 848), "VOCAL", "vocal", 1, 24.0, R.art_vo, 62, readout="DE-ESS")
    panel((674, 572, 1132, 848), "COLOUR", "colour", 1, 24.0, R.art_sat, 62, chip=True)
    R.chain_column(img, R.RAIL, active=("EQ",))


def page_image():
    R.portrait_rail(img)
    R.header(img, "IMAGE")
    R.footer(img)
    panel((212, 70, 820, 560), "STEREO IMAGER", "img", 1, 48.0, R.art_fan, 170, readout="M/S 92")
    R.analyser_plate(img, (832, 70, 1132, 560), stats=True)
    panel((212, 572, 662, 848), "WIDTH PER BAND", "imgband", 1, 24.0, None, 0, readout="92 Hz")
    panel((674, 572, 1132, 848), "COLOUR", "colour", 1, 24.0, R.art_sat, 62, chip=True)
    R.chain_column(img, R.RAIL, active=("IMAGER",))


PAGES = {"all": page_all, "eq": page_eq, "image": page_image}

if __name__ == "__main__":
    name = sys.argv[1] if len(sys.argv) > 1 else "all"
    PAGES[name]()
    R.finish(img, "cpp-" + name)
