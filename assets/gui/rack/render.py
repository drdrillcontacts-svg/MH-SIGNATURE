#!/usr/bin/env python3
"""The ten minimal directions.  Run: python3 render.py"""
import math
import os
import sys

from PIL import ImageDraw

from kit import (W, H, NOIR, PANEL, PANEL2, EDGE, EDGE2, ACC, DEEP, SOFT, INK, INK2,
                 MUT, DIM, px, rr, line, circle, text, pill, knob, spectrum, meter,
                 paste_portrait, header, footer, signature, new_board)

OUT = os.path.dirname(os.path.abspath(__file__))

# ------------------------------------------------------------------ module data
MODULES = [
    ("EQ",       "eq",      [("HPF", "32 Hz", 0.10), ("LOW", "+3.0", 0.58),
                             ("LO MID", "-2.4", 0.42), ("HI MID", "-3.5", 0.38),
                             ("HIGH", "-4.0", 0.34)]),
    ("DYNAMICS", "dyn",     [("THRESH", "-18.0", 0.55), ("RATIO", "4.0:1", 0.32),
                             ("ATTACK", "10 ms", 0.22), ("RELEASE", "100", 0.30),
                             ("MAKEUP", "+3.2", 0.26)]),
    ("VOCAL",    "vocal",   [("HPF", "95 Hz", 0.24), ("BODY", "+2.0", 0.58),
                             ("DE-ESS", "6.5 k", 0.42), ("PRES", "-1.5", 0.44),
                             ("AIR", "-6.0", 0.30)]),
    ("COLOR",    "color",   [("DRIVE", "4.5", 0.20), ("TONE", "37%", 0.37),
                             ("MIX", "58%", 0.58)]),
    ("DELAY",    "delay",   [("TIME", "375 ms", 0.19), ("FDBK", "36%", 0.36),
                             ("MIX", "18%", 0.18)]),
    ("REVERB",   "reverb",  [("DECAY", "3.4 s", 0.27), ("DAMP", "70%", 0.70),
                             ("MIX", "24%", 0.24)]),
    ("IMAGE",    "image",   [("WIDTH", "132%", 0.66), ("BAL", "0", 0.50)]),
    ("MODULATION", "mod",   [("RATE", "0.30", 0.12), ("DEPTH", "28%", 0.28),
                             ("MIX", "16%", 0.16)]),
    ("TRANSIENT", "trn",    [("ATTACK", "+12", 0.56), ("SUSTAIN", "-8", 0.46),
                             ("SPLIT", "180 Hz", 0.30)]),
    ("I/O",      "io",      [("INPUT", "-3.2", 0.43), ("OUTPUT", "-1.0", 0.48),
                             ("MIX", "100%", 1.00)]),
]
MODNAME = {m[1]: m[0] for m in MODULES}
BYKEY = {m[1]: m for m in MODULES}


def art(d, kind, box):
    """A 1-px line display in each card, the same language as the plugin."""
    x0, y0, x1, y1 = box
    w, h = x1 - x0, y1 - y0
    if h < 8:
        return

    if kind == "eq":
        line(d, x0, (y0 + y1) / 2, x1, (y0 + y1) / 2, (30, 34, 40), 1)
        pts = []
        for i in range(60):
            t = i / 59.0
            f = 20 * 1000 ** t
            db = (3.0 * math.exp(-0.5 * (math.log2(f / 65) / 1.15) ** 2)
                  - 2.4 * math.exp(-0.5 * (math.log2(f / 152) / 0.85) ** 2)
                  - 3.5 * math.exp(-0.5 * (math.log2(f / 3500) / 0.95) ** 2)
                  - 4.0 * math.exp(-0.5 * (math.log2(f / 9000) / 1.5) ** 2)
                  - (t - 0.5) * 6)
            pts.append((px(x0 + t * w), px((y0 + y1) / 2 - db / 16 * h)))
        d.line(pts, fill=SOFT, width=max(1, px(1.3)))

    elif kind == "dyn":
        pts = [(px(x0 + i / 30 * w), px(y1 - (min(i / 30 * 40, 40 - max(0, i / 30 * 40 - 18) * 0.75)) / 40 * h))
               for i in range(31)]
        d.line(pts, fill=ACC, width=max(1, px(1.2)))
        text(d, (x1, y0 + 2), "-6.8 dB", 8.0, INK2, anchor="ra", mono=True)

    elif kind == "color":
        pts = []
        for i in range(70):
            t = i / 69.0
            pts.append((px(x0 + t * w), px((y0 + y1) / 2 - math.tanh(math.sin(t * 6.28 * 3) * 2.2) * h * 0.44)))
        d.line(pts, fill=SOFT, width=max(1, px(1.3)))

    elif kind == "reverb":
        pts = [(px(x0 + i / 50 * w), px(y1 - math.exp(-i / 50 * 2.4) * h)) for i in range(51)]
        d.line(pts, fill=SOFT, width=max(1, px(1.3)))

    elif kind == "delay":
        for c in range(8):
            a = 0.9 ** c
            for r in range(2):
                circle(d, x0 + c / 7 * w, y0 + h * (0.3 + 0.4 * r), 1.4 + 1.2 * a,
                       fill=ACC if (c + r) % 2 == 0 else DEEP)

    elif kind == "mod":
        pts = [(px(x0 + i / 60 * w), px((y0 + y1) / 2 - math.sin(i / 60 * 6.28) * h * 0.42)) for i in range(61)]
        d.line(pts, fill=SOFT, width=max(1, px(1.3)))

    elif kind == "image":
        cx, cy = (x0 + x1) / 2, y1
        r = min(w * 0.45, h)
        d.arc([px(cx - r), px(cy - r), px(cx + r), px(cy + r)], 180, 360, fill=(40, 44, 51), width=max(1, px(1.2)))
        d.pieslice([px(cx - r * 0.96), px(cy - r * 0.96), px(cx + r * 0.96), px(cy + r * 0.96)],
                   240, 300, fill=(58, 20, 14))

    elif kind == "trn":
        pts = [(px(x0 + i / 50 * w), px(y1 - math.exp(-i / 50 * 4.2) * h)) for i in range(51)]
        d.line(pts, fill=SOFT, width=max(1, px(1.3)))


def module_card(board, d, box, name, key, knobs, art_h=34, knob_r=20, title_size=11.0,
                power_w=44, art_kind=None, compact=False):
    x0, y0, x1, y1 = box
    rr(d, box, radius=7, fill=(17, 20, 25), outline=EDGE, width=1)

    text(d, (x0 + 13, y0 + 15), name, title_size, (230, 235, 241), bold=True, anchor="lm", track=1.6)
    pill(d, (x1 - 12 - power_w, y0 + 6, x1 - 12, y0 + 24), True)

    top = y0 + 30
    if art_kind and art_h > 0:
        art(d, art_kind, (x0 + 13, top, x1 - 13, top + art_h))
        top += art_h + 8

    n = len(knobs)
    if n == 0:
        return
    slot = (x1 - x0 - 20) / n
    r = min(knob_r, slot / 2 - 6, (y1 - top - 26) / 2)
    cy = top + r + 2
    for i, (lab, val, frac) in enumerate(knobs):
        knob(d, x0 + 10 + slot * (i + 0.5), cy, r, lab, val, frac)


# =========================================================================== 01
def design_01(board, d):
    """QUIET GRID — one calm grid, nothing but type and hairlines."""
    header(board, d, portrait_tile=((24, 10, 54, 44),),
           tabs_at=(516, 12, 372, 30), preset=(904, 8, 1170, 48))
    line(d, 0, 62, W, 62, (24, 27, 33), 1)
    meter(d, (24, 74, 74, 850), -3.2, "in")
    meter(d, (1366, 74, 1416, 850), -1.0, "out")

    # engine + spectrum
    eb = (86, 74, 700, 288)
    rr(d, eb, radius=8, fill=(18, 21, 26), outline=EDGE, width=1)
    text(d, (100, 92), "SIGNATURE ENGINE", 11.5, (230, 235, 241), bold=True, anchor="lm", track=1.8)
    text(d, (686, 92), "TURN IDEAS INTO SOUND", 8.4, DIM, anchor="rm", track=2.4)
    for i, (lab, val, frac) in enumerate([("CHARACTER", "62", 0.62), ("DENSITY", "48", 0.48),
                                          ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56)]):
        knob(d, 150 + (i % 2) * 110, 152 + (i // 2) * 92, 26, lab, val, frac, label_size=9.4, value_size=8.8)
    knob(d, 560, 180, 68, "INTENSITY", "84%", 0.84, big=True)
    spectrum(d, (716, 74, 1416, 288))

    cards = [("EQ", "eq"), ("DYNAMICS", "dyn"), ("VOCAL", "vocal"), ("COLOR", "color")]
    cw = (1416 - 24 - 86 - 3 * 12) / 4
    for i, (name, key) in enumerate(cards):
        x = 86 + i * (cw + 12)
        module_card(board, d, (x, 300, x + cw, 486), name, key,
                    BYKEY[key][2], art_h=30, knob_r=20)

    cards = [("DELAY", "delay"), ("REVERB", "reverb"), ("IMAGE", "image"), ("MODULATION", "mod")]
    for i, (name, key) in enumerate(cards):
        x = 86 + i * (cw + 12)
        module_card(board, d, (x, 498, x + cw, 684), name, key,
                    BYKEY[key][2], art_h=28, knob_r=20)

    wide = (cw * 2 + 12)
    module_card(board, d, (86, 696, 86 + wide, 850), "TRANSIENT", "trn",
                BYKEY["trn"][2], art_h=24, knob_r=22)
    module_card(board, d, (98 + wide, 696, 98 + wide * 2, 850), "I/O", "io",
                BYKEY["io"][2], art_h=0, knob_r=22)

    sb = (110 + wide * 2, 696, 1416, 850)
    rr(d, sb, radius=7, fill=(14, 16, 20), outline=EDGE, width=1)
    paste_portrait(board, (sb[0] + 1, sb[1] + 1, sb[0] + 106, sb[3] - 1), radius=6, fade_bottom=0.0,
                   fade_right=0.35, tint=0.6)
    signature(d, sb[0] + 190, sb[1] + 56, 34)
    text(d, (sb[0] + 190, sb[1] + 96), "MUSIC / PEOPLE / EMOTION", 8.2, INK2, anchor="mm", track=2.4)
    text(d, (sb[0] + 190, sb[1] + 112), "ALWAYS", 8.2, MUT, anchor="mm", track=2.4)

    footer(d)


# =========================================================================== 02
def design_02(board, d):
    """PORTRAIT RAIL — the portrait becomes the left wall of the instrument."""
    paste_portrait(board, (0, 0, 330, 900), fade_bottom=0.42, fade_right=0.22, tint=0.55, dim=0.88)
    line(d, 330, 0, 330, 900, (28, 31, 37), 1)

    header(board, d, tabs_at=(706, 12, 372, 30), preset=(1092, 8, 1416, 48), portrait_tile=None)
    # the wordmark sits over the portrait
    text(d, (24, 22), "MH", 24, INK, bold=True, anchor="lm")
    line(d, 76, 12, 76, 44, (60, 52, 48), 1)

    signature(d, 165, 300, 46)
    text(d, (165, 372), "MUSIC / PEOPLE / EMOTION", 8.6, (222, 216, 210), anchor="mm", track=2.6)
    text(d, (165, 390), "ALWAYS", 8.6, (186, 180, 175), anchor="mm", track=2.6)
    knob(d, 118, 520, 44, "CHARACTER", "62", 0.62, big=True)
    knob(d, 232, 520, 44, "DENSITY", "48", 0.48, big=True)
    knob(d, 118, 668, 44, "SPACE", "70", 0.70, big=True)
    knob(d, 232, 668, 44, "WIDTH", "56", 0.56, big=True)
    knob(d, 165, 800, 52, "INTENSITY", "84%", 0.84, big=True)

    meter(d, (342, 68, 392, 856), -3.2, "in")
    meter(d, (1354, 68, 1404, 856), -1.0, "out")

    spectrum(d, (404, 68, 1040, 236))
    rr(d, (1052, 68, 1340, 236), radius=7, fill=(18, 21, 26), outline=EDGE, width=1)
    text(d, (1066, 86), "GAIN REDUCTION", 9.0, MUT, bold=True, anchor="lm", track=1.8)
    rr(d, (1066, 100, 1326, 118), radius=3, fill=(10, 12, 15))
    rr(d, (1066, 100, 1160, 118), radius=3, fill=ACC)
    text(d, (1326, 152), "-6.8 dB", 26, INK, anchor="rm", mono=True)
    text(d, (1066, 168), "LIMITER CEILING  -0.3 dBFS", 8.0, DIM, anchor="lm", track=1.6)

    xs = [404, 720, 1036]
    cw = 304
    rows = [(252, 430, [("EQ", "eq"), ("DYNAMICS", "dyn"), ("VOCAL", "vocal")]),
            (442, 620, [("COLOR", "color"), ("DELAY", "delay"), ("REVERB", "reverb")]),
            (632, 810, [("IMAGE", "image"), ("MODULATION", "mod"), ("TRANSIENT", "trn")])]

    for y0, y1, trio in rows:
        for i, (name, key) in enumerate(trio):
            module_card(board, d, (xs[i], y0, xs[i] + cw, y1), name, key,
                        BYKEY[key][2], art_h=26, knob_r=19)

    # I/O rides under the engine knobs of the right column
    module_card(board, d, (1052, 632, 1340, 810), "I/O", "io", BYKEY["io"][2],
                art_h=0, knob_r=22)
    footer(d, left="VERSION 1.0.0")


# =========================================================================== 03
def design_03(board, d):
    """LEDGER — ten rows, no cards, nothing but the chain."""
    header(board, d, tabs_at=(690, 12, 372, 30), preset=(1080, 8, 1416, 48))
    line(d, 0, 62, W, 62, (24, 27, 33), 1)

    paste_portrait(board, (24, 78, 300, 468), radius=6, fade_bottom=0.45, tint=0.6)
    signature(d, 162, 500, 38)
    text(d, (162, 552), "MUSIC / PEOPLE / EMOTION", 8.2, INK2, anchor="mm", track=2.2)
    text(d, (162, 568), "ALWAYS", 8.2, MUT, anchor="mm", track=2.2)

    text(d, (24, 606), "INPUT", 8.4, MUT, anchor="lm", track=1.8)
    meter(d, (24, 618, 74, 850), -3.2, "in")
    text(d, (150, 606), "OUTPUT", 8.4, MUT, anchor="lm", track=1.8)
    meter(d, (150, 618, 200, 850), -1.0, "out")

    x0, x1 = 320, 1416
    top, rowh = 74, 78
    for i, (name, key, knobs) in enumerate(MODULES):
        y = top + i * rowh
        if i % 2 == 0:
            rr(d, (x0, y, x1, y + rowh - 8), radius=5, fill=(13, 15, 19))
        line(d, x0, y + rowh - 8, x1, y + rowh - 8, (24, 27, 33), 1)

        text(d, (x0 + 18, y + 34), f"{i + 1:02d}", 9.0, (90, 97, 108), anchor="lm", mono=True)
        text(d, (x0 + 52, y + 30), name, 12.5, (232, 237, 243), bold=True, anchor="lm", track=1.8)
        text(d, (x0 + 52, y + 46), "IN CHAIN", 7.6, MUT, anchor="lm", track=1.8)
        pill(d, (x0 + 176, y + 22, x0 + 232, y + 44), True, size=8.4)

        n = len(knobs)
        for k, (lab, val, frac) in enumerate(knobs):
            cx = x0 + 300 + k * 118
            knob(d, cx, y + 33, 17, lab, val, frac, label_size=8.0, value_size=7.6)

        text(d, (x1 - 24, y + 34), "ON", 8.4, ACC, anchor="rm", track=1.6)

    footer(d)


# =========================================================================== 04
def design_04(board, d):
    """SPLIT — portrait and engine own the left half, the rack owns the right."""
    paste_portrait(board, (0, 0, 640, 340), fade_bottom=0.52, fade_right=0.5, tint=0.55)

    text(d, (28, 30), "MH", 24, INK, bold=True, anchor="lm")
    line(d, 80, 20, 80, 52, (70, 62, 58), 1)
    text(d, (100, 30), "MH SIGNATURE", 16, INK, anchor="lm", track=3.0)
    text(d, (101, 50), "BY MILAD HASHEMI", 8.4, MUT, anchor="lm", track=2.4)

    signature(d, 150, 252, 40)
    text(d, (150, 312), "MUSIC / PEOPLE / EMOTION", 8.6, (226, 220, 214), anchor="mm", track=2.6)
    text(d, (150, 330), "ALWAYS", 8.6, (190, 184, 178), anchor="mm", track=2.6)

    # engine block
    rr(d, (24, 366, 616, 500), radius=8, fill=(17, 20, 25), outline=EDGE, width=1)
    text(d, (38, 384), "SIGNATURE ENGINE", 11.0, INK, bold=True, anchor="lm", track=1.8)
    for i, (lab, val, frac) in enumerate([("CHARACTER", "62", 0.62), ("DENSITY", "48", 0.48),
                                          ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56)]):
        knob(d, 82 + i * 86, 436, 21, lab, val, frac, label_size=7.8, value_size=7.4)
    knob(d, 548, 438, 38, "INTENSITY", "84%", 0.84, big=True, label_size=8.6, value_size=8.4)

    # left meters
    meter(d, (24, 516, 74, 850), -3.2, "in")
    meter(d, (566, 516, 616, 850), -1.0, "out")

    # chain status list — the same ten modules, with their ON state
    x0, x1 = 96, 552
    text(d, (x0, 530), "CHAIN STATUS", 8.0, MUT, bold=True, anchor="lm", track=2.2)
    text(d, (x1, 530), "10 / 10 ACTIVE", 8.0, ACC, anchor="rm", track=1.6)
    for i, (name, key, _) in enumerate(MODULES):
        y = 552 + i * 28
        text(d, (x0 + 4, y + 10), f"{i + 1:02d}", 7.6, (86, 93, 104), anchor="lm", mono=True)
        text(d, (x0 + 32, y + 10), name, 9.4, (216, 221, 228), anchor="lm", track=1.4)
        line(d, x0 + 150, y + 10, x1 - 60, y + 10, (30, 34, 40), 1)
        circle(d, x1 - 34, y + 10, 2.6, fill=ACC)
        text(d, (x1 - 4, y + 10), "ON", 7.6, MUT, anchor="rm", track=1.2)

    # right side chrome only — no second wordmark
    labels = ["ALL", "EQ", "DYN", "COLOR", "SPACE", "MOD", "IMAGE"]
    tx, ty, tw_all, th = 690, 12, 372, 30
    tw = tw_all / len(labels)
    rr(d, (tx, ty, tx + tw_all, ty + th), radius=th / 2, fill=(11, 13, 17),
       outline=(30, 34, 41), width=1)
    for i, lab in enumerate(labels):
        bx = tx + i * tw
        if i == 0:
            rr(d, (bx + 3, ty + 3, bx + tw - 3, ty + th - 3), radius=(th - 6) / 2, fill=(178, 45, 23))
        text(d, (bx + tw / 2, ty + th / 2), lab, 8.2, (250, 226, 219) if i == 0 else MUT,
             bold=True, anchor="mm", track=1.6)

    rr(d, (1080, 8, 1416, 48), radius=20, fill=(17, 20, 25), outline=(34, 38, 45), width=1)
    text(d, (1096, 20), "SIGNATURE PRESET · 001", 7.4, (140, 148, 160), anchor="lm", track=1.6)
    text(d, (1248, 35), "Modern Clarity", 12.6, INK, anchor="mm")

    spectrum(d, (650, 66, 1416, 186))

    for i, (name, key, knobs) in enumerate(MODULES):
        col = i % 2
        row = i // 2
        x = 650 + col * 388
        y = 198 + row * 132
        box = (x, y, x + 376, y + 122)
        rr(d, box, radius=6, fill=(15, 18, 22), outline=EDGE, width=1)
        text(d, (x + 12, y + 16), name, 10.6, (230, 235, 241), bold=True, anchor="lm", track=1.4)
        pill(d, (x + 300, y + 7, x + 364, y + 25), True, size=8.0)
        for k, (lab, val, frac) in enumerate(knobs):
            knob(d, x + 40 + k * 68, y + 48, 15, lab, val, frac, label_size=7.6, value_size=7.2)

    footer(d)


# =========================================================================== 05
def design_05(board, d):
    """FADERS — a console: ten strips, one per module."""
    header(board, d, portrait_tile=((24, 10, 54, 44),), tabs_at=(516, 12, 372, 30),
           preset=(904, 8, 1170, 48))
    line(d, 0, 62, W, 62, (24, 27, 33), 1)

    # engine strip
    rr(d, (24, 70, 1416, 172), radius=8, fill=(17, 20, 25), outline=EDGE, width=1)
    text(d, (40, 92), "SIGNATURE ENGINE", 11.4, INK, bold=True, anchor="lm", track=1.8)
    for i, (lab, val, frac) in enumerate([("CHARACTER", "62", 0.62), ("DENSITY", "48", 0.48),
                                          ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56),
                                          ("INTENSITY", "84", 0.84)]):
        knob(d, 320 + i * 128, 122, 26, lab, val, frac)
    spectrum(d, (1000, 78, 1400, 164), detail=False)

    # ten strips
    x0, x1 = 24, 1416
    n = len(MODULES)
    sw = (x1 - x0 - 9 * 8) / n
    for i, (name, key, knobs) in enumerate(MODULES):
        sx = x0 + i * (sw + 8)
        rr(d, (sx, 184, sx + sw, 806), radius=6, fill=(14, 17, 21), outline=EDGE, width=1)
        text(d, (sx + sw / 2, 202), name, 9.4, (232, 237, 243), bold=True, anchor="mm", track=1.4)
        pill(d, (sx + 8, 214, sx + sw - 8, 236), True, size=7.8)

        for k, (lab, val, frac) in enumerate(knobs):
            nk = max(1, len(knobs))
            spread = 18.0 if nk >= 5 else 34.0
            fx = sx + sw / 2 + (k - (nk - 1) / 2.0) * spread
            fy0, fy1 = 296, 640
            line(d, fx, fy0, fx, fy1, (32, 36, 43), 3)
            line(d, fx, fy1 - (fy1 - fy0) * frac, fx, fy1, ACC, 3)
            ry = fy1 - (fy1 - fy0) * frac
            rr(d, (fx - 7, ry - 5, fx + 7, ry + 5), radius=2.5, fill=(226, 221, 217))
            text(d, (fx, ry - 15), val, 6.8, SOFT, anchor="mm", mono=True)
            text(d, (fx, fy1 + 18), lab.split(" ")[0], 6.2, MUT, anchor="mm", track=0.2)

        text(d, (sx + sw / 2, 782), f"{i + 1:02d}", 8.0, (92, 99, 110), anchor="mm", mono=True)

    meter(d, (24, 816, 82, 858), -3.2, "in")
    meter(d, (1358, 816, 1416, 858), -1.0, "out")
    footer(d, left=None)


# =========================================================================== 06
def design_06(board, d):
    """HERO DIAL — one enormous control, the rack at the edge."""
    header(board, d, tabs_at=(516, 12, 372, 30), preset=(1100, 8, 1416, 48))
    line(d, 0, 62, W, 62, (24, 27, 33), 1)

    paste_portrait(board, (24, 78, 300, 420), radius=8, fade_bottom=0.5, tint=0.6)
    signature(d, 162, 452, 34)
    text(d, (162, 496), "MUSIC / PEOPLE / EMOTION", 8.0, INK2, anchor="mm", track=2.0)

    spectrum(d, (24, 520, 300, 700))
    meter(d, (24, 712, 300, 850), -3.2, "in")

    cx, cy = 640, 400
    circle(d, cx, cy, 168, outline=(30, 34, 40), width=1)
    knob(d, cx, cy, 150, "INTENSITY", "84%", 0.84, big=True, label_size=13, value_size=13)
    for i, (lab, val, frac) in enumerate([("CHARACTER", "62", 0.62), ("DENSITY", "48", 0.48),
                                          ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56)]):
        a = math.radians(-135 + i * 90)
        knob(d, cx + math.cos(a) * 250, cy + math.sin(a) * 235, 34, lab, val, frac)

    text(d, (cx, 726), "TURN IDEAS INTO SOUND", 9.0, DIM, anchor="mm", track=3.0)

    # rack list
    x0, x1 = 1010, 1416
    text(d, (x0, 88), "THE CHAIN", 9.0, MUT, bold=True, anchor="lm", track=2.4)
    for i, (name, key, knobs) in enumerate(MODULES):
        y = 100 + i * 76
        rr(d, (x0, y, x1, y + 66), radius=5, fill=(14, 17, 21) if i % 2 else (17, 20, 25),
           outline=EDGE, width=1)
        text(d, (x0 + 14, y + 22), name, 10.4, (232, 237, 243), bold=True, anchor="lm", track=1.4)
        pill(d, (x0 + 316, y + 8, x1 - 10, y + 28), True, size=8.0)
        vals = "   ".join(f"{k[0]} {k[1]}" for k in knobs[:3])
        text(d, (x0 + 14, y + 46), vals, 7.8, MUT, anchor="lm")

    meter(d, (958, 356, 990, 700), -1.0, "out")
    footer(d)


# =========================================================================== 07
def design_07(board, d):
    """STACK — an accordion rack, one column, very quiet."""
    header(board, d, tabs_at=(470, 12, 372, 30), preset=(1050, 8, 1416, 48),
           portrait_tile=((24, 10, 54, 44),))
    line(d, 0, 62, W, 62, (24, 27, 33), 1)

    x0, x1 = 24, 1090
    for i, (name, key, knobs) in enumerate(MODULES):
        y = 74 + i * 78
        rr(d, (x0, y, x1, y + 68), radius=6, fill=(15, 18, 22), outline=EDGE, width=1)
        if i == 0:
            rr(d, (x0, y, x0 + 3, y + 68), radius=1.5, fill=ACC)
        text(d, (x0 + 20, y + 24), name, 11.6, (232, 237, 243), bold=True, anchor="lm", track=1.8)
        text(d, (x0 + 20, y + 46), "MODULE " + f"{i + 1:02d}", 7.6, DIM, anchor="lm", track=1.8)
        pill(d, (x0 + 150, y + 23, x0 + 208, y + 45), True, size=8.2)
        for k, (lab, val, frac) in enumerate(knobs):
            knob(d, x0 + 300 + k * 126, y + 34, 18, lab, val, frac, label_size=8.2, value_size=7.8)

    # right rail
    paste_portrait(board, (1104, 74, 1416, 372), radius=8, fade_bottom=0.35, tint=0.6)
    signature(d, 1260, 410, 34)
    text(d, (1260, 452), "MUSIC / PEOPLE / EMOTION", 7.8, INK2, anchor="mm", track=2.0)
    spectrum(d, (1104, 474, 1416, 660))
    meter(d, (1104, 672, 1230, 850), -3.2, "in")
    meter(d, (1290, 672, 1416, 850), -1.0, "out")
    footer(d)


# =========================================================================== 08
def design_08(board, d):
    """SPLIT DARK — the portrait bleeds through the top right corner."""
    paste_portrait(board, (860, 0, 1440, 470), fade_bottom=0.55, fade_left=0.45, tint=0.5)

    # header chrome on the dark side only
    labels = ["ALL", "EQ", "DYN", "COLOR", "SPACE", "MOD", "IMAGE"]
    tx, ty, tw_all, th = 24, 12, 372, 30
    tw = tw_all / len(labels)
    rr(d, (tx, ty, tx + tw_all, ty + th), radius=th / 2, fill=(11, 13, 17),
       outline=(30, 34, 41), width=1)
    for i, lab in enumerate(labels):
        bx = tx + i * tw
        if i == 0:
            rr(d, (bx + 3, ty + 3, bx + tw - 3, ty + th - 3), radius=(th - 6) / 2, fill=(178, 45, 23))
        text(d, (bx + tw / 2, ty + th / 2), lab, 8.2, (250, 226, 219) if i == 0 else MUT,
             bold=True, anchor="mm", track=1.6)

    text(d, (28, 76), "MH SIGNATURE", 15.0, INK, anchor="lm", track=3.0)
    text(d, (29, 96), "BY MILAD HASHEMI", 8.0, MUT, anchor="lm", track=2.4)
    rr(d, (24, 112, 330, 152), radius=20, fill=(17, 20, 25), outline=(34, 38, 45), width=1)
    text(d, (40, 124), "SIGNATURE PRESET · 001", 7.2, (140, 148, 160), anchor="lm", track=1.4)
    text(d, (177, 140), "Modern Clarity", 12.2, INK, anchor="mm")

    text(d, (1416, 402), "MUSIC / PEOPLE / EMOTION", 8.6, (240, 234, 228), anchor="rm", track=2.6)
    text(d, (1416, 422), "ALWAYS", 8.6, (208, 202, 197), anchor="rm", track=2.6)
    signature(d, 1300, 458, 42)

    # engine
    rr(d, (24, 170, 640, 330), radius=8, fill=(16, 19, 24), outline=EDGE, width=1)
    text(d, (40, 188), "SIGNATURE ENGINE", 11.4, INK, bold=True, anchor="lm", track=1.8)
    for i, (lab, val, frac) in enumerate([("CHARACTER", "62", 0.62), ("DENSITY", "48", 0.48),
                                          ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56)]):
        knob(d, 92 + i * 94, 242, 25, lab, val, frac, label_size=8.4, value_size=8.0)
    knob(d, 560, 246, 46, "INTENSITY", "84%", 0.84, big=True)

    spectrum(d, (96, 344, 640, 560))
    meter(d, (24, 500, 74, 848), -3.2, "in")

    # I/O card lives in the left column
    rr(d, (96, 606, 640, 726), radius=7, fill=(15, 18, 22), outline=EDGE, width=1)
    text(d, (110, 626), "I/O", 10.6, (230, 235, 241), bold=True, anchor="lm", track=1.4)
    pill(d, (566, 616, 628, 634), True, size=8.0)
    for k, (lab, val, frac) in enumerate(BYKEY["io"][2]):
        knob(d, 176 + k * 130, 668, 17, lab, val, frac, label_size=7.8, value_size=7.4)

    text(d, (96, 780), "10 MODULES · 100 FACTORY PRESETS · 75 PARAMETERS",
         8.0, (92, 100, 112), anchor="lm", track=2.0)

    # module grid bottom (I/O already placed above)
    for i, (name, key, knobs) in enumerate(MODULES[:9]):
        col = i % 3
        row = i // 3
        x = 656 + col * 240
        y = 504 + row * 116
        box = (x, y, x + 232, y + 104)
        rr(d, box, radius=6, fill=(14, 17, 21), outline=EDGE, width=1)
        text(d, (x + 14, y + 20), name, 10.2, (230, 235, 241), bold=True, anchor="lm", track=1.4)
        pill(d, (x + 366, y + 9, x + 426, y + 27), True, size=7.8)
        for k, (lab, val, frac) in enumerate(knobs[:4]):
            knob(d, x + 30 + k * 57, y + 52, 14, lab, val, frac, label_size=6.8, value_size=6.6)

    meter(d, (1372, 506, 1414, 848), -1.0, "out")
    footer(d)


# =========================================================================== 09
def design_09(board, d):
    """BLUEPRINT — hairlines only, the accent is the only colour."""
    header(board, d, tabs_at=(516, 12, 372, 30), preset=(1104, 8, 1416, 48))
    # grid
    for x in range(0, W, 48):
        line(d, x, 62, x, 866, (19, 22, 27), 1)
    for y in range(62, 900, 48):
        line(d, 0, y, W, y, (19, 22, 27), 1)
    line(d, 0, 62, W, 62, (34, 38, 45), 1)

    paste_portrait(board, (24, 74, 264, 354), radius=4, fade_bottom=0.4, tint=0.6)
    signature(d, 144, 396, 32)
    text(d, (144, 434), "MUSIC / PEOPLE / EMOTION", 7.6, INK2, anchor="mm", track=2.0)

    rr(d, (24, 456, 264, 640), radius=4, fill=(11, 13, 17), outline=(34, 38, 45), width=1)
    text(d, (38, 474), "ENGINE", 9.0, ACC, bold=True, anchor="lm", track=2.2)
    for i, (lab, val, frac) in enumerate([("CHARACTER", "62", 0.62), ("DENSITY", "48", 0.48),
                                          ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56)]):
        text(d, (38, 502 + i * 22), lab, 8.0, MUT, anchor="lm", track=1.4)
        line(d, 120, 502 + i * 22, 240, 502 + i * 22, (36, 40, 47), 2)
        line(d, 120, 502 + i * 22, 120 + 120 * frac, 502 + i * 22, ACC, 2)
        text(d, (248, 502 + i * 22), val, 8.0, SOFT, anchor="rm", mono=True)
    knob(d, 144, 600, 30, "INTENSITY", "84%", 0.84, label_size=8.4, value_size=8.0)

    meter(d, (24, 656, 264, 850), -3.2, "in")

    spectrum(d, (288, 74, 1416, 226), detail=True)

    for i, (name, key, knobs) in enumerate(MODULES):
        col = i % 4
        row = i // 4
        x = 288 + col * 276
        y = 240 + row * 210
        box = (x, y, x + 262, y + 198)
        rr(d, box, radius=3, fill=None, outline=(40, 45, 53), width=1)
        text(d, (x + 14, y + 20), name, 10.0, (236, 240, 246), bold=True, anchor="lm", track=1.6)
        line(d, x + 14, y + 30, x + 258, y + 30, (40, 45, 53), 1)
        pill(d, (x + 208, y + 10, x + 258, y + 28), True, size=7.6)
        for k, (lab, val, frac) in enumerate(knobs[:4]):
            knob(d, x + 36 + k * 63, y + 80, 16, lab, val, frac, label_size=7.2, value_size=7.0)
        text(d, (x + 14, y + 178), "ACTIVE", 7.0, (74, 82, 94), anchor="lm", track=1.6)

    meter(d, (1372, 240, 1414, 848), -1.0, "out")
    footer(d)


# =========================================================================== 10
def design_10(board, d):
    """CONSOLE — two columns of wide channel cards, everything on one line."""
    header(board, d, tabs_at=(516, 12, 372, 30), preset=(1104, 8, 1416, 48),
           portrait_tile=((24, 10, 54, 44),))
    line(d, 0, 62, W, 62, (24, 27, 33), 1)

    # engine band
    rr(d, (24, 72, 1416, 200), radius=9, fill=(18, 21, 26), outline=EDGE, width=1)
    paste_portrait(board, (24, 72, 200, 200), radius=9, fade_bottom=0.0, fade_right=0.25, tint=0.6)
    text(d, (214, 100), "SIGNATURE ENGINE", 12.0, INK, bold=True, anchor="lm", track=2.2)
    text(d, (214, 122), "TURN IDEAS INTO SOUND", 8.4, DIM, anchor="lm", track=2.4)
    for i, (lab, val, frac) in enumerate([("CHARACTER", "62", 0.62), ("DENSITY", "48", 0.48),
                                          ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56)]):
        knob(d, 272 + i * 64, 152, 19, lab, val, frac, label_size=7.8, value_size=7.4)
    knob(d, 600, 136, 44, "INTENSITY", "84%", 0.84, big=True)
    spectrum(d, (700, 82, 1110, 190), detail=False)
    meter(d, (1126, 84, 1200, 190), -3.2, "in")
    meter(d, (1216, 84, 1290, 190), -1.0, "out")
    signature(d, 1370, 130, 26)
    text(d, (1370, 168), "ALWAYS", 7.4, MUT, anchor="mm", track=2.0)

    for i, (name, key, knobs) in enumerate(MODULES):
        col = i % 2
        row = i // 2
        x = 24 + col * 704
        y = 214 + row * 128
        box = (x, y, x + 688, y + 118)
        rr(d, box, radius=7, fill=(14, 17, 21), outline=EDGE, width=1)
        rr(d, (x, y, x + 3, y + 118), radius=1.5, fill=ACC if i == 0 else (52, 56, 64))
        text(d, (x + 18, y + 22), name, 11.0, (232, 237, 243), bold=True, anchor="lm", track=1.6)
        text(d, (x + 18, y + 40), f"{i + 1:02d}", 7.4, DIM, anchor="lm", mono=True)
        pill(d, (x + 18, y + 56, x + 76, y + 78), True, size=8.0)
        for k, (lab, val, frac) in enumerate(knobs[:4]):
            knob(d, x + 130 + k * 96, y + 52, 22, lab, val, frac, label_size=8.2, value_size=7.8)
        art(d, key, (x + 520, y + 26, x + 668, y + 96))

    footer(d)


DESIGNS = [
    ("01", "QUIET GRID", design_01, "یک شبکهٔ آرام و منظم؛ همه کارت‌ها هم‌اندازه، تایپ درشت، بدون شلوغی"),
    ("02", "PORTRAIT RAIL", design_02, "پرتره به‌عنوان دیوار چپ ساز؛ ماژول‌ها در سه ستون سمت راست"),
    ("03", "LEDGER", design_03, "ده ردیف پشت‌سرهم، بدون کارت؛ مثل یک دفتر ثبت زنجیرهٔ صدا"),
    ("04", "SPLIT", design_04, "نیمهٔ چپ پرتره و موتور، نیمهٔ راست رک؛ کنتراست دو نیمه"),
    ("05", "FADERS", design_05, "کنسول: ده نوار عمودی، هر ماژول یک کانال با فیدرهای واقعی"),
    ("06", "HERO DIAL", design_06, "یک نُب عظیم مرکزی؛ رک به‌صورت فهرست باریک در لبه"),
    ("07", "STACK", design_07, "یک ستون آکاردئونی؛ هر ماژول یک نوار با سه نُب درون‌خطی"),
    ("08", "SPLIT DARK", design_08, "پرتره از گوشهٔ بالا سرریز می‌کند؛ ماژول‌ها پایین در سه ستون"),
    ("09", "BLUEPRINT", design_09, "فقط خط مو؛ نقشهٔ مهندسی با لهجهٔ قرمز به‌جای رنگ"),
    ("10", "CONSOLE", design_10, "دو ستون کارت پهن؛ همه‌چیز روی یک خط، نوار موتور بالای صفحه"),
]


def main():
    os.makedirs(OUT, exist_ok=True)
    for num, name, fn, _ in DESIGNS:
        board = new_board()
        d = ImageDraw.Draw(board)
        fn(board, d)
        path = os.path.join(OUT, f"rack-{num}-{name.lower().replace(' ', '-')}.png")
        board.save(path, optimize=True)
        print("wrote", os.path.basename(path), board.size)


if __name__ == "__main__":
    main()
