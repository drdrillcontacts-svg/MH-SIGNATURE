"""Preview of the C++ editor geometry.

PluginEditor.cpp lays every plate out with the same numbers this script uses, so
this is the quickest way to look at a page without opening a DAW.  The art
primitives come from render_v3.py — they are the ones the C++ was ported from.

Run:  python3 assets/gui/rack/render_cpp_layout.py
Out:  assets/gui/rack/v3-chat-mock-all.jpg
"""

import os, sys, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import render_v3 as R
from PIL import ImageDraw

INK, INK2, MUT, DIM, FAINT, ACC, SOFT = R.INK, R.INK2, R.MUT, R.DIM, R.FAINT, R.ACC, R.SOFT

D = {
 "engine": [("INTENSITY","100",1.0)],
 "eq": [("HPF","32 Hz",.07),("LOW","+3.0",.58),("LO MID","-2.4",.43),("HI MID","-3.5",.40),("HIGH","-4.0",.39)],
 "dyn": [("THRESH","-18.0",.70),("RATIO","4.0:1",.16),("ATTACK","10 ms",.09),
         ("RELEASE","100 ms",.09),("MAKEUP","+3.2",.13),("CEILING","-0.3",.96)],
 "colour": [("DRIVE","4.5",.19),("TONE","37%",.37),("MIX","58%",.58)],
 "vocal": [("HPF","95 Hz",.26),("BODY","+2.0",.58),("DE-ESS","6.5 k",.42),("PRES","-1.5",.44),("AIR","-6.0",.33)],
 "reverb": [("DECAY","3.4 s",.27),("PREDLY","26 ms",.13),("DAMP","70%",.70),("MIX","24%",.24)],
 "mod": [("RATE","0.30",.05),("DEPTH","28%",.28),("MIX","16%",.16)],
 "motion": [("RATE","0.30",.05),("DEPTH","28%",.28),("MIX","16%",.16),
            ("ATTACK","+12",.56),("SUSTAIN","-8",.46),("SPLIT","180 Hz",.07)],
 "trns": [("ATTACK","+12",.56),("SUSTAIN","-8",.46),("SPLIT","180 Hz",.07)],
}

img = R.new_board()

def rocker(x, y, on, w, h, label=True):
    R.toggle(img, x, y, on, w, h, label=label)

def panel(box, title, key, rows=1, max_r=22, art=None, art_h=0, chip=None,
          chip_component=False, sections=None, readout="", powers=(("", True),)):
    R.plate(img, box, "", True)
    d = ImageDraw.Draw(img)
    x0, y0, x1, y1 = box
    w, h = x1 - x0, y1 - y0

    R.text(d, (x0 + 30, y0 + 20), title.upper(), 9.4, INK, track=2.4, anchor="lm")

    n = len(powers)
    px_ = x1 - 30
    for name, on in reversed(powers):
        single = (n == 1)
        ww = 84 if single else 44
        px_ -= ww
        if single:
            rocker(px_, y0 + 10, on, 44, 18, True)
        else:
            rocker(px_, y0 + 11, on, 30, 16, False)
        px_ -= 0 if single else 8

    if readout:
        right = x1 - 30 - (0 if n == 0 else (90 if n == 1 else 100))
        R.text(d, (right, y0 + 20), readout, 6.8, DIM, track=1.2, anchor="rm")

    R.dline(d, x0 + 26, y0 + 36, x1 - 26, y0 + 36, (10, 11, 13), 1.0)
    R.dline(d, x0 + 26, y0 + 37, x1 - 26, y0 + 37, (44, 48, 55), 0.9)

    top = 46
    if chip_component:
        cw = min(w - 52, 132)
        d.rounded_rectangle([R.px(x0 + 26), R.px(y0 + top), R.px(x0 + 26 + cw), R.px(y0 + top + 18)],
                            radius=R.px(2.5), fill=R.LED_BG, outline=(44, 48, 55), width=R.px(0.9))
        R.text(d, (x0 + 33, y0 + top + 9), "CHARACTER", 6.0, DIM, track=1.5, anchor="lm")
        R.text(d, (x0 + 26 + cw - 7, y0 + top + 9), "CONSOLE", 6.6, SOFT, track=0.8, anchor="rm", mono=True)
        top += 26

    if sections:
        secs = len(sections)
        rowH = (h - 8 - 46) // secs
        per = math.ceil(len(D[key]) / secs)
        for i, lab in enumerate(sections):
            R.text(d, (x0 + 30, y0 + 46 + rowH * i + 8), lab, 6.2, DIM, track=2.2, anchor="lm")
            if i:
                R.dline(d, x0 + 26, y0 + 46 + rowH * i - 2, x1 - 26, y0 + 46 + rowH * i - 2, (12, 13, 16), 1.0)
        for i, (lab, val, frac) in enumerate(D[key]):
            sec, ci = i // per, i % per
            cnt = min(per, len(D[key]) - sec * per)
            slotW = (w - 32) // cnt
            boxH = rowH - 18
            r = max(8.0, min((slotW - 26) / 2.0, (boxH - 56) / 2.0, max_r))
            bh = 2 * round(r) + 44
            cx = x0 + 16 + slotW * (ci + 0.5)
            cy = y0 + 46 + rowH * sec + 16 + (boxH - bh) / 2 + round(r)
            R.dial(img, cx, cy, round(r), lab, val, frac,
                   label_size=min(7.6, r * 0.44), value_size=min(7.0, r * 0.40))
        return

    if art is not None and art_h > 0:
        art(d, (x0 + 26, y0 + top, x1 - 26, y0 + top + art_h))
        top += art_h + 12

    boxH = h - 8 - top
    per = math.ceil(len(D[key]) / rows)
    slotW = (w - 32) // per
    slotH = boxH // rows
    r = max(8.0, min((slotW - 26) / 2.0, (slotH - 56) / 2.0, max_r))
    bh = 2 * round(r) + 44
    for i, (lab, val, frac) in enumerate(D[key]):
        ri, ci = i // per, i % per
        cx = x0 + 16 + slotW * (ci + 0.5)
        cy = y0 + top + slotH * ri + (slotH - bh) / 2 + round(r)
        R.dial(img, cx, cy, round(r), lab, val, frac,
               label_size=min(7.6, r * 0.44), value_size=min(7.0, r * 0.40))

# ------------------------------------------------------------------ frame
R.portrait_rail(img)
R.header(img, "ALL")
R.footer(img)

# ------------------------------------------------------------------ engine
box = (212, 70, 752, 300)
R.plate(img, box, "", True)
d = ImageDraw.Draw(img)
R.text(d, (box[0] + 30, box[1] + 20), "SIGNATURE ENGINE", 9.8, INK, track=3.4, anchor="lm")
R.text(d, (box[2] - 30, box[1] + 20), "TURN IDEAS INTO SOUND", 6.2, DIM, track=2.2, anchor="rm")
R.dline(d, box[0] + 26, box[1] + 36, box[2] - 26, box[1] + 36, (10, 11, 13), 1.0)
R.dline(d, box[0] + 26, box[1] + 37, box[2] - 26, box[1] + 37, (44, 48, 55), 0.9)
cx = box[0] + 270
R.dial(img, cx, box[1] + 128, 48, "INTENSITY", "100%", 1.0, label_size=7.6, value_size=7.0)
for lab, val, frac, mx, my in (("CHARACTER", "62", .62, 45, 46), ("DENSITY", "48", .48, 45, 142),
                               ("SPACE", "70", .70, 433, 46), ("WIDTH", "56", .56, 433, 142)):
    R.dial(img, box[0] + mx + 31, box[1] + my + 18, 18, lab, val, frac,
           label_size=6.6, value_size=6.2)

R.analyser_plate(img, (764, 70, 1132, 300), stats=False)

panel((212, 312, 510, 582), "EQ", "eq", 1, 22.0, R.art_eq, 54, readout="4 BANDS")
panel((522, 312, 822, 582), "DYNAMICS", "dyn", 2, 22.0, None, 0, readout="GR -6.8 dB",
      powers=(("COMP", True), ("LIMIT", True)))
panel((834, 312, 1132, 582), "COLOUR", "colour", 1, 22.0, R.art_sat, 74, chip_component=True)
panel((212, 594, 510, 848), "VOCAL", "vocal", 1, 22.0, R.art_vo, 44, readout="DE-ESS 6.5K")
panel((522, 594, 822, 848), "REVERB", "reverb", 1, 22.0, R.art_decay, 40, readout="3.4 s",
      powers=(("REVERB", True), ("DELAY", True)))
panel((834, 594, 1132, 848), "MOTION", "motion", 2, 17.0, None, 0,
      sections=("MODULATION", "TRANSIENT"), powers=(("MOD", True), ("TRNS", True)))

R.chain_column(img, R.RAIL, active=("ENGINE",))

R.finish(img, "mock-all")
