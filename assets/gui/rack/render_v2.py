#!/usr/bin/env python3
"""Five directions from scratch, each in a different visual language."""
import math
import os

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageOps

from kit import (W, H, NOIR, PANEL, EDGE, ACC, DEEP, SOFT, INK, INK2, MUT, DIM,
                 px, S, rr, line, circle, text, pill, knob, spectrum, meter,
                 paste_portrait, footer, signature, font,
                 F_SANS, F_BOLD, F_SERIF, F_MONO)

HERE = os.path.dirname(os.path.abspath(__file__))

MODULES = [
    ("EQ",        [("HPF", "32 Hz", 0.10), ("LOW", "+3.0", 0.58), ("LO MID", "-2.4", 0.42),
                   ("HI MID", "-3.5", 0.38), ("HIGH", "-4.0", 0.34)]),
    ("DYNAMICS",  [("THRESH", "-18.0", 0.55), ("RATIO", "4.0:1", 0.32), ("ATTACK", "10 ms", 0.22),
                   ("RELEASE", "100 ms", 0.30), ("MAKEUP", "+3.2", 0.26)]),
    ("VOCAL",     [("HPF", "95 Hz", 0.24), ("BODY", "+2.0", 0.58), ("DE-ESS", "6.5 k", 0.42),
                   ("PRES", "-1.5", 0.44), ("AIR", "-6.0", 0.30)]),
    ("COLOR",     [("DRIVE", "4.5", 0.20), ("TONE", "37%", 0.37), ("MIX", "58%", 0.58)]),
    ("DELAY",     [("TIME", "375 ms", 0.19), ("FDBK", "36%", 0.36), ("MIX", "18%", 0.18)]),
    ("REVERB",    [("DECAY", "3.4 s", 0.27), ("DAMP", "70%", 0.70), ("MIX", "24%", 0.24)]),
    ("IMAGE",     [("WIDTH", "132%", 0.66), ("BAL", "0", 0.50)]),
    ("MODULATION", [("RATE", "0.30", 0.12), ("DEPTH", "28%", 0.28), ("MIX", "16%", 0.16)]),
    ("TRANSIENT", [("ATTACK", "+12", 0.56), ("SUSTAIN", "-8", 0.46), ("SPLIT", "180 Hz", 0.30)]),
    ("I/O",       [("INPUT", "-3.2", 0.43), ("OUTPUT", "-1.0", 0.48), ("MIX", "100%", 1.00)]),
]
BY = {m[0]: m[1] for m in MODULES}


def mono(xy, s, size, colour=INK2, anchor="lm"):
    return text_anchor(xy, s, size, colour, anchor, mono=True)


def text_anchor(xy, s, size, colour=INK2, anchor="lm", mono=False, serif=False, bold=False):
    d = _d
    path = F_MONO if mono else (F_SERIF if serif else (F_BOLD if bold else F_SANS))
    f = font(path, size)
    d.text((px(xy[0]), px(xy[1])), s, font=f, fill=colour, anchor=anchor)


_d = None


def knob_row(box, knobs, top=44, bottom=16, r=20, label=None, value=None):
    """Knobs centred in the band that is actually free inside a tile."""
    x0, y0, x1, y1 = box
    band_t, band_b = y0 + top, y1 - bottom
    cy = band_t + (band_b - band_t) * 0.42
    n = max(1, len(knobs))
    step = (x1 - x0 - 32) / n
    for k, (lab, val, frac) in enumerate(knobs):
        cx = x0 + 16 + step * (k + 0.5)
        knob(_d, cx, cy, r, lab, val, frac,
             label_size=label or (7.4 if n > 3 else 8.0),
             value_size=value or (7.0 if n > 3 else 7.6))


def led_pill(box, on=True):
    """Minimal switch: a ring, a filled core, and the word."""
    x, y, x1, y1 = box
    cy = (y + y1) / 2
    r = min((y1 - y) / 2, 3.6)
    if on:
        circle(_d, x + r, cy, r * 2.0, fill=(58, 20, 13))
        circle(_d, x + r, cy, r, fill=ACC)
    else:
        circle(_d, x + r, cy, r, outline=(84, 88, 96), width=1)
    text(_d, (x + r * 3.2, cy), "ON" if on else "OFF", 7.0,
         SOFT if on else (108, 112, 118), anchor="lm", mono=True)


# ======================================================================= 21
#  CALIBRATION — a measuring instrument.  Dense ticks, mono numerals, no cards.
def design_21(board, d):
    global _d
    _d = d
    board.paste((8, 9, 10), (0, 0, W * S, H * S))

    # frame
    line(d, 16, 16, W - 16, 16, (38, 40, 44), 1)
    line(d, 16, H - 16, W - 16, H - 16, (38, 40, 44), 1)
    line(d, 16, 16, 16, H - 16, (38, 40, 44), 1)
    line(d, W - 16, 16, W - 16, H - 16, (38, 40, 44), 1)

    text_anchor((34, 34), "MH SIGNATURE", 13, INK, "lm", bold=True)
    text_anchor((34, 50), "SIGNATURE ANALYSER · MODEL MHS-1", 7.6, DIM, "lm", mono=True)
    text_anchor((W - 34, 34), "SERIAL 0100-22", 7.6, DIM, "rm", mono=True)
    text_anchor((W - 34, 50), "V 1.0.0", 7.6, DIM, "rm", mono=True)

    for i, lab in enumerate(["ALL", "EQ", "DYN", "COLOR", "SPACE", "MOD", "IMAGE"]):
        x = 380 + i * 62
        if i == 0:
            rr(d, (x - 16, 26, x + 46, 48), radius=2, fill=(24, 12, 9), outline=(96, 30, 18), width=1)
            text_anchor((x + 15, 37), lab, 8.6, SOFT, "mm", mono=True)
        else:
            text_anchor((x + 15, 37), lab, 8.6, DIM, "mm", mono=True)

    # reference block: portrait as a specimen with its own measurement
    pb = (34, 74, 274, 356)
    paste_portrait(board, pb, radius=0, fade_bottom=0.15, tint=0.0, dim=0.92)
    rr(d, pb, radius=0, outline=(46, 48, 52), width=1)
    text_anchor((34, 368), "REF · M. HASHEMI", 7.4, DIM, "lm", mono=True)
    text_anchor((274, 368), "T60 3.40 s", 7.4, DIM, "rm", mono=True)

    # big dial, with a vernier scale
    cx, cy, r = 154, 468, 74
    for t in range(0, 101, 2):
        a = math.radians(135 + t * 2.7)
        ln = 8 if t % 10 == 0 else 4
        line(d, cx + math.cos(a) * (r + 6), cy + math.sin(a) * (r + 6),
             cx + math.cos(a) * (r + 6 + ln), cy + math.sin(a) * (r + 6 + ln),
             (94, 96, 102) if t % 10 == 0 else (54, 56, 62), 1)
    knob(d, cx, cy, r, "INTENSITY", "84.0 %", 0.84, big=True, label_size=9.4, value_size=9.0)

    for i, (lab, val, frac) in enumerate([("CHARACTER", "62", 0.62), ("DENSITY", "48", 0.48),
                                          ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56)]):
        bx = 34 + i * 61
        knob(d, bx + 26, 600, 20, lab, val, frac, label_size=7.2, value_size=7.0)

    # session measurements, two columns, under the macro row
    line(_d, 34, 690, 300, 690, (34, 36, 40), 1)
    text(_d, (34, 706), "SESSION", 7.0, DIM, "lm", mono=True)
    for i, (k, v) in enumerate([("LUFS", "−7.6"), ("PEAK", "−0.3"),
                                ("T60", "3.40 s"), ("SIDE/MID", "−7.7"),
                                ("TILT", "−5.4"), ("LRA", "5.2")]):
        col, row = i % 2, i // 2
        x = 34 + col * 138
        y = 728 + row * 40
        text(_d, (x, y), k, 6.8, (104, 106, 112), "lm", mono=True)
        text(_d, (x, y + 15), v, 11.0, (214, 218, 224), "lm", mono=True)
        line(_d, x, y + 24, x + 118, y + 24, (26, 28, 32), 1)

    # main plot
    plot = (330, 74, 1406, 520)
    rr(d, plot, radius=0, fill=(6, 7, 8), outline=(40, 42, 46), width=1)
    text_anchor((342, 88), "MAGNITUDE  ·  dBFS  ·  20 Hz — 20 kHz", 7.4, DIM, "lm", mono=True)

    inner = (346, 104, 1392, 496)
    for db in range(0, -61, -6):
        y = inner[1] + (0 - db) / 60 * (inner[3] - inner[1])
        line(d, inner[0], y, inner[2], y, (24, 26, 30) if db % 12 == 0 else (17, 19, 22), 1)
        if db % 12 == 0:
            text_anchor((340, y), f"{db}", 6.8, (104, 106, 112), "rm", mono=True)

    for f in [20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000]:
        x = inner[0] + math.log(f / 20) / math.log(1000) * (inner[2] - inner[0])
        line(d, x, inner[1], x, inner[3], (22, 24, 28), 1)
        text_anchor((x, 505), f"{f//1000}k" if f >= 1000 else str(f), 6.6, (100, 102, 108), "mm", mono=True)

    rng = np.random.default_rng(11)
    pts = []
    for i in range(150):
        t = i / 149
        v = -22 + 5 * math.sin(t * 2.0) - 17 * t + 10 * math.exp(-((t - .21) ** 2) / .004) \
            - 8 * math.exp(-((t - .54) ** 2) / .01) + rng.normal(0, 1.4)
        x = inner[0] + t * (inner[2] - inner[0])
        y = inner[3] - (max(-60, min(0, v)) + 60) / 60 * (inner[3] - inner[1])
        pts.append((px(x), px(y)))
    d.line(pts, fill=SOFT, width=max(1, px(1.1)))

    text_anchor((1406, 88), "PEAK −0.3   LUFS −7.6   LRA 5.2", 7.2, (140, 142, 148), "rm", mono=True)

    # module grid: rows of readouts, no boxes
    y0 = 540
    for i, (name, knobs) in enumerate(MODULES):
        col = i % 5
        row = i // 5
        x = 330 + col * 216
        y = y0 + row * 148
        text_anchor((x, y), name, 8.6, (218, 222, 228), "lm", mono=True)
        led_pill((x + 4, y + 10, x + 12, y + 18), True)
        for k, (lab, val, frac) in enumerate(knobs[:4]):
            kx = x + 18 + k * 48
            knob(d, kx, y + 58, 14, lab, val, frac, label_size=6.4, value_size=6.2)

    text_anchor((W / 2, 876), "AN ALL-IN-ONE MIXING ENVIRONMENT   ·   DESIGNED BY MILAD HASHEMI",
                8.0, (120, 122, 128), "mm", mono=True)


# ======================================================================= 22
#  FOLIO — an editorial spread.  A masthead, a huge portrait, an index of modules.
def design_22(board, d):
    global _d
    _d = d
    board.paste((11, 11, 12), (0, 0, W * S, H * S))

    # left page: the portrait, full bleed to the gutter
    paste_portrait(board, (0, 0, 640, 900), fade_bottom=0.10, fade_right=0.30, tint=0.0, dim=0.96)
    line(d, 640, 0, 640, 900, (44, 30, 26), 1)

    # right page
    rr(d, (676, 46, 1404, 48), radius=0, fill=(70, 44, 36))

    text_anchor((676, 84), "MH SIGNATURE", 40, INK, "lm", serif=True)
    text_anchor((676, 122), "by Milad Hashemi", 15, (198, 162, 150), "lm", serif=True)

    text_anchor((676, 164), "AN ALL-IN-ONE MIXING ENVIRONMENT", 8.2, (150, 132, 126), "lm", mono=True)
    text_anchor((676, 180), "ISSUE 01 · TEN MODULES · ONE ENGINE", 8.2, (150, 132, 126), "lm", mono=True)

    # the folio: engine
    text_anchor((676, 236), "INTENSITY", 9.0, (208, 176, 166), "lm", mono=True)
    text_anchor((676, 300), "84", 66, SOFT, "lm", serif=True)
    text_anchor((772, 262), "CHARACTER 62", 8.4, (186, 174, 168), "lm", mono=True)
    text_anchor((772, 280), "DENSITY 48", 8.4, (186, 174, 168), "lm", mono=True)
    text_anchor((772, 298), "SPACE 70", 8.4, (186, 174, 168), "lm", mono=True)
    text_anchor((772, 316), "WIDTH 56", 8.4, (186, 174, 168), "lm", mono=True)

    for i, (lab, val, frac) in enumerate([("CHARACTER", "62", 0.62), ("DENSITY", "48", 0.48),
                                          ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56)]):
        knob(d, 940 + i * 84, 288, 26, lab, val, frac, label_size=7.0, value_size=6.8)

    text_anchor((676, 344), "TURN IDEAS INTO SOUND", 7.4, (140, 126, 120), "lm", mono=True)

    # the index of ten modules — one line each, two columns
    line(d, 676, 366, 1404, 366, (54, 40, 34), 1)
    text_anchor((676, 382), "CONTENTS", 8.0, (150, 132, 126), "lm", mono=True)
    text_anchor((1404, 382), "ALL ACTIVE", 8.0, ACC, "rm", mono=True)

    for i, (name, knobs) in enumerate(MODULES):
        col = i // 5
        row = i % 5
        x = 676 + col * 372
        y = 408 + row * 88

        text_anchor((x, y), f"{i+1:02d}", 8.6, (150, 124, 116), "lm", mono=True)
        text_anchor((x + 28, y), name, 12.0, (232, 228, 224), "lm", serif=True)
        led_pill((x + 182, y - 5, x + 190, y + 3), True)

        vals = "   ".join(f"{k[0].split()[0]} {k[1]}" for k in knobs[:4])
        text_anchor((x + 28, y + 18), vals, 7.0, (156, 146, 140), "lm", mono=True)

        line(d, x, y + 40, x + 344, y + 40, (40, 30, 26), 1)

    # colophon
    line(d, 676, 856, 1404, 856, (54, 40, 34), 1)
    text_anchor((676, 874), "VERSION 1.0.0", 7.4, (130, 116, 110), "lm", mono=True)
    text_anchor((1040, 874), "DESIGNED BY MILAD HASHEMI", 7.4, (130, 116, 110), "mm", mono=True)
    text_anchor((1404, 874), "100 PRESETS", 7.4, (130, 116, 110), "rm", mono=True)


# ======================================================================= 23
#  RACK UNIT — engraved hardware. Flat grey plates, no gradients, pure typography.
def design_23(board, d):
    global _d
    _d = d
    board.paste((10, 10, 11), (0, 0, W * S, H * S))

    PLATE = (34, 36, 40)
    ENGRAVE = (176, 180, 186)

    # brand plate
    rr(d, (24, 20, 1416, 92), radius=3, fill=PLATE, outline=(58, 61, 66), width=1)
    paste_portrait(board, (26, 22, 118, 90), radius=2, fade_bottom=0.0, fade_right=0.2, dim=0.95)
    text_anchor((132, 44), "MH SIGNATURE", 16, (238, 240, 243), "lm", bold=True)
    text_anchor((132, 62), "ALL-IN-ONE MIXING ENVIRONMENT", 7.4, ENGRAVE, "lm", mono=True)
    text_anchor((132, 76), "MODEL MHS-1  ·  MILAD HASHEMI", 7.0, (128, 132, 138), "lm", mono=True)

    for i, lab in enumerate(["ALL", "EQ", "DYN", "COLOR", "SPACE", "MOD", "IMAGE"]):
        x = 640 + i * 54
        text_anchor((x, 44), lab, 7.8, SOFT if i == 0 else (120, 124, 130), "lm", mono=True)
        if i == 0:
            line(d, x, 54, x + 22, 54, ACC, 2)

    # preset window
    rr(d, (1080, 30, 1320, 80), radius=2, fill=(18, 19, 22), outline=(58, 61, 66), width=1)
    text_anchor((1092, 44), "PRESET 001", 7.0, (120, 124, 130), "lm", mono=True)
    text_anchor((1092, 62), "Modern Clarity", 11.4, (232, 234, 238), "lm")

    text_anchor((1332, 44), "SOUND", 7.0, (120, 124, 130), "lm", mono=True)
    text_anchor((1332, 58), "WITH", 7.0, (120, 124, 130), "lm", mono=True)
    text_anchor((1332, 72), "INTENTION", 7.0, (120, 124, 130), "lm", mono=True)

    # engine plate, wide
    rr(d, (24, 108, 1416, 300), radius=3, fill=PLATE, outline=(58, 61, 66), width=1)
    text_anchor((44, 128), "SIGNATURE ENGINE", 9.6, ENGRAVE, "lm", mono=True)
    text_anchor((1396, 128), "TURN IDEAS INTO SOUND", 7.2, (120, 124, 130), "rm", mono=True)

    knob(d, 200, 214, 54, "INTENSITY", "84 %", 0.84, big=True, label_size=9.0, value_size=9.0)
    for i, (lab, val, frac) in enumerate([("CHARACTER", "62", 0.62), ("DENSITY", "48", 0.48),
                                          ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56)]):
        knob(d, 360 + i * 112, 204, 30, lab, val, frac, label_size=8.0, value_size=7.6)

    # thin analyser strip inside the plate
    strip = (820, 176, 1396, 268)
    rr(d, strip, radius=2, fill=(9, 10, 12), outline=(48, 51, 56), width=1)
    rng = np.random.default_rng(5)
    pts = []
    for i in range(90):
        t = i / 89
        v = -20 - 18 * t + 9 * math.exp(-((t - .2) ** 2) / .006) + rng.normal(0, 1.6)
        pts.append((px(strip[0] + 8 + t * (strip[2] - strip[0] - 16)),
                    px(strip[3] - 10 - (max(-58, min(-2, v)) + 58) / 56 * (strip[3] - strip[1] - 20))))
    d.line(pts, fill=SOFT, width=max(1, px(1.1)))

    # ten plates
    for i, (name, knobs) in enumerate(MODULES):
        col = i % 5
        row = i // 5
        x = 24 + col * 280
        y = 316 + row * 282
        box = (x, y, x + 268, y + 266)
        rr(d, box, radius=3, fill=PLATE, outline=(58, 61, 66), width=1)

        text_anchor((x + 14, y + 22), name, 9.0, (232, 234, 238), "lm", mono=True)
        pill(d, (x + 204, y + 13, x + 256, y + 30), True, size=6.8)
        line(d, x + 14, y + 36, x + 254, y + 36, (52, 55, 60), 1)

        knob_row(box, knobs[:4], top=50, bottom=128, r=26, label=7.4, value=7.0)

        # engraved value rail: a scale per knob, no repeated text
        strip_top = y + 196
        line(d, x + 14, strip_top - 14, x + 254, strip_top - 14, (50, 53, 58), 1)
        n = max(1, len(knobs[:4]))
        for k, (lab, val, frac) in enumerate(knobs[:4]):
            cx = x + 14 + (240 / n) * (k + 0.5)
            line(d, cx - 22, strip_top + 6, cx + 22, strip_top + 6, (46, 49, 54), 3)
            line(d, cx - 22, strip_top + 6, cx - 22 + 44 * frac, strip_top + 6, ACC, 3)
            text_anchor((cx, strip_top + 26), f"{int(frac * 100):02d}", 6.4, (110, 114, 120), "mm", mono=True)
        text_anchor((x + 254, strip_top + 42), "IN CHAIN", 6.0, (96, 100, 106), "rm", mono=True)

    # meters on the bottom edge
    text_anchor((24, 872), "INPUT  −3.2 dB", 7.6, ENGRAVE, "lm", mono=True)
    text_anchor((1416, 872), "OUTPUT  −1.0 dB", 7.6, ENGRAVE, "rm", mono=True)
    for i in range(40):
        on = i < 27
        x = 150 + i * 6
        line(d, x, 866, x, 878, ACC if on else (52, 55, 60), 3)
    for i in range(40):
        on = i < 30
        x = 1070 + i * 6
        line(d, x, 866, x, 878, ACC if on else (52, 55, 60), 3)


# ======================================================================= 24
#  BROADSIDE — poster. Giant numerals, halftone portrait, modules as tickets.
def design_24(board, d):
    global _d
    _d = d
    board.paste((7, 8, 9), (0, 0, W * S, H * S))

    # halftone portrait block, right column
    pbox = (900, 0, 1440, 520)
    paste_portrait(board, pbox, radius=0, fade_bottom=0.55, fade_left=0.10, dim=1.0)

    # vertical masthead
    for i, ch in enumerate("MHSIGNATURE"):
        y = 60 + i * 46
        text_anchor((70, y), ch, 34, INK if i < 2 else (60, 62, 66), "lm", bold=True)

    text_anchor((150, 76), "MH SIGNATURE", 15, INK, "lm", bold=True)
    text_anchor((150, 96), "BY MILAD HASHEMI", 7.6, (150, 154, 160), "lm", mono=True)

    for i, lab in enumerate(["ALL", "EQ", "DYN", "COLOR", "SPACE", "MOD", "IMAGE"]):
        x = 150 + i * 56
        text_anchor((x, 126), lab, 7.4, SOFT if i == 0 else (110, 114, 120), "lm", mono=True)
        if i == 0:
            line(d, x, 136, x + 24, 136, ACC, 2)

    # the number
    text_anchor((150, 196), "84", 128, (238, 241, 245), "lm", bold=True)
    text_anchor((392, 140), "INTENSITY", 12, SOFT, "lm", mono=True)
    text_anchor((392, 164), "CHARACTER 62", 9.0, (170, 174, 180), "lm", mono=True)
    text_anchor((392, 182), "DENSITY 48", 9.0, (170, 174, 180), "lm", mono=True)
    text_anchor((392, 200), "SPACE 70", 9.0, (170, 174, 180), "lm", mono=True)
    text_anchor((392, 218), "WIDTH 56", 9.0, (170, 174, 180), "lm", mono=True)
    text_anchor((392, 246), "TURN IDEAS INTO SOUND", 8.2, (120, 124, 130), "lm", mono=True)

    knob(d, 196, 320, 44, "CHARACTER", "62", 0.62, label_size=8.4, value_size=8.2)
    knob(d, 300, 320, 44, "DENSITY", "48", 0.48, label_size=8.4, value_size=8.2)
    knob(d, 404, 320, 44, "SPACE", "70", 0.70, label_size=8.4, value_size=8.2)
    knob(d, 508, 320, 44, "WIDTH", "56", 0.56, label_size=8.4, value_size=8.2)

    # meters in the right column, poster style
    for j, (label, level, x) in enumerate([("IN", -3.2, 980), ("OUT", -1.0, 1204)]):
        text_anchor((x, 556), label, 8.0, (150, 154, 160), "lm", mono=True)
        text_anchor((x, 574), f"{level:.1f}", 15, (232, 236, 242), "lm", bold=True)
        for i in range(22):
            on = i < (19 if j == 0 else 21)
            line(d, x + i * 6, 596, x + i * 6, 606, ACC if on else (40, 42, 46), 4)

    text_anchor((980, 640), "THE SESSION", 8.0, (120, 124, 130), "lm", mono=True)
    for i, (k, v) in enumerate([("LUFS", "−7.6"), ("PEAK", "−0.3"), ("T60", "3.40 s"),
                                ("SIDE/MID", "−7.7 dB"), ("PRESETS", "100")]):
        y = 662 + i * 26
        text_anchor((1340, y), k, 7.6, (120, 124, 130), "rm", mono=True)
        text_anchor((980, y), v, 8.4, (206, 210, 216), "lm", mono=True)
        line(d, 980, y + 12, 1340, y + 12, (30, 32, 36), 1)

    # caption over the portrait
    text_anchor((1420, 480), "MUSIC / PEOPLE / EMOTION", 9.0, (248, 242, 236), "rm", mono=True)
    text_anchor((1420, 498), "ALWAYS", 9.0, (214, 208, 202), "rm", mono=True)
    signature(d, 1180, 430, 40)

    # tickets
    text_anchor((150, 404), "THE CHAIN — TEN MODULES", 7.6, (120, 124, 130), "lm", mono=True)
    for i, (name, knobs) in enumerate(MODULES):
        col = i % 2
        row = i // 2
        x = 150 + col * 366
        y = 424 + row * 88
        rr(d, (x, y, x + 344, y + 78), radius=0, fill=(13, 14, 17), outline=(36, 38, 42), width=1)
        text_anchor((x + 12, y + 20), f"{i+1:02d}", 8.0, (108, 112, 118), "lm", mono=True)
        text_anchor((x + 38, y + 20), name, 9.4, (230, 234, 240), "lm", mono=True)
        pill(d, (x + 286, y + 10, x + 334, y + 26), True, size=6.4)

        vals = "   ".join(f"{k[0]} {k[1]}" for k in knobs[:3])
        text_anchor((x + 38, y + 42), vals, 7.2, (150, 154, 160), "lm", mono=True)

        # value bars instead of knobs — the poster language
        for k, (lab, val, frac) in enumerate(knobs[:4]):
            bx = x + 38 + k * 74
            line(d, bx, y + 60, bx + 56, y + 60, (38, 40, 44), 2)
            line(d, bx, y + 60, bx + 56 * frac, y + 60, ACC, 2)

    line(d, 150, 862, 1416, 862, (36, 38, 42), 1)
    text_anchor((150, 878), "VERSION 1.0.0", 7.4, (120, 124, 130), "lm", mono=True)
    text_anchor((1416, 878), "DESIGNED BY MILAD HASHEMI", 7.4, (120, 124, 130), "rm", mono=True)


# ======================================================================= 25
#  BENTO — unequal tiles, generous air, one big analyser.
def design_25(board, d):
    global _d
    _d = d
    board.paste((9, 10, 12), (0, 0, W * S, H * S))

    def tile(box, radius=12, fill=(17, 19, 23), outline=(33, 36, 42)):
        rr(d, box, radius=radius, fill=fill, outline=outline, width=1)
        return box

    def title(box, name, on=True):
        text_anchor((box[0] + 20, box[1] + 24), name, 10.6, (232, 236, 242), "lm", bold=True)
        pill(d, (box[2] - 82, box[1] + 15, box[2] - 20, box[1] + 33), on, size=7.4)

    # header
    text_anchor((28, 34), "MH", 22, INK, "lm", bold=True)
    text_anchor((28, 58), "SIGNATURE", 8.4, (150, 154, 160), "lm", mono=True)

    for i, lab in enumerate(["ALL", "EQ", "DYN", "COLOR", "SPACE", "MOD", "IMAGE"]):
        x = 300 + i * 58
        if i == 0:
            rr(d, (x - 14, 24, x + 44, 46), radius=11, fill=(30, 14, 10))
            text_anchor((x + 15, 35), lab, 7.8, SOFT, "mm", mono=True)
        else:
            text_anchor((x + 15, 35), lab, 7.8, (118, 122, 128), "mm", mono=True)

    rr(d, (1040, 18, 1412, 56), radius=19, fill=(16, 18, 22), outline=(34, 37, 43), width=1)
    text_anchor((1060, 31), "PRESET 001", 6.8, (128, 132, 138), "lm", mono=True)
    text_anchor((1060, 46), "Modern Clarity", 11.6, INK, "lm")

    # rows of the bento
    # engine (wide) + portrait + analyser (tall)
    eng = tile((24, 78, 560, 344))
    title(eng, "SIGNATURE ENGINE")
    knob(d, 474, 210, 60, "INTENSITY", "84%", 0.84, big=True, label_size=9.6, value_size=9.4)
    for i, (lab, val, frac) in enumerate([("CHARACTER", "62", 0.62), ("DENSITY", "48", 0.48),
                                          ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56)]):
        knob(d, 96 + (i % 2) * 104, 164 + (i // 2) * 96, 28, lab, val, frac,
             label_size=7.8, value_size=7.4)
    text_anchor((44, 320), "TURN IDEAS INTO SOUND", 7.4, (108, 112, 118), "lm", mono=True)

    por = tile((572, 78, 792, 344), fill=(12, 13, 16))
    paste_portrait(board, (573, 79, 791, 343), radius=11, fade_bottom=0.22, tint=0.0, dim=0.98)
    signature(d, 682, 300, 26)
    text_anchor((682, 330), "ALWAYS", 6.6, (170, 164, 158), "mm", mono=True)

    ana = tile((804, 78, 1416, 344))
    title(ana, "ANALYSER")
    spectrum(d, (816, 108, 1404, 332))

    # module tiles
    small = [
        ("EQ",        (24, 356, 472, 560)),
        ("DYNAMICS",  (484, 356, 932, 560)),
        ("VOCAL",     (944, 356, 1416, 560)),
        ("COLOR",     (24, 572, 352, 776)),
        ("DELAY",     (364, 572, 692, 776)),
        ("REVERB",    (704, 572, 1032, 776)),
        ("IMAGE",     (1044, 572, 1416, 776)),
        ("MODULATION", (24, 782, 352, 852)),
        ("TRANSIENT", (364, 782, 692, 852)),
        ("I/O",       (704, 782, 1416, 852)),
    ]

    for name, box in small:
        tile(box, radius=10)
        title(box, name)
        knobs = BY[name]
        if len(knobs) <= 3:
            knob_row(box, knobs, top=48, bottom=18, r=24, label=8.0, value=7.6)
        else:
            knob_row(box, knobs[:5], top=48, bottom=18, r=21, label=7.0, value=6.8)

    text_anchor((W / 2, 880), "AN ALL-IN-ONE MIXING ENVIRONMENT  ·  VERSION 1.0.0  ·  DESIGNED BY MILAD HASHEMI",
                7.0, (96, 100, 106), "mm", mono=True)


DESIGNS = [
    ("21", "CALIBRATION", design_21,
     "زبان ابزار اندازه‌گیری: ترسیم‌های تیک‌دار، اعداد مونواسپیس، بدون کارت — همه‌چیز مثل یک دستگاه کالیبره"),
    ("22", "FOLIO", design_22,
     "زبان مجله: پرترهٔ تمام‌قد در صفحهٔ چپ، مَست‌هد سریف، ماژول‌ها به‌صورت فهرست مطالب شماره‌دار"),
    ("23", "RACK UNIT", design_23,
     "زبان سخت‌افزار: صفحات خاکستری تخت با نوشتهٔ حکاکی‌شده، ده پلیت هم‌اندازه، بدون هیچ گرادیانی"),
    ("24", "BROADSIDE", design_24,
     "زبان پوستر: عدد ۸۴ به‌اندازهٔ پوستر، پرترهٔ هافتون، ماژول‌ها به‌شکل تیکت با نوار مقدار"),
    ("25", "BENTO", design_25,
     "زبان مدرن: شبکهٔ بنتو با کاشی‌های نامساوی، فضای خالی زیاد و یک آنالایزر بزرگ"),
]


def main():
    for num, name, fn, _ in DESIGNS:
        board = Image.new("RGB", (W * S, H * S), NOIR)
        d = ImageDraw.Draw(board)
        fn(board, d)
        path = os.path.join(HERE, f"gui-{num}-{name.lower().replace(' ', '-')}.png")
        board.save(path, optimize=True)
        print("wrote", os.path.basename(path))


if __name__ == "__main__":
    main()
