#!/usr/bin/env python3
"""MH SIGNATURE — v3, "analogue desk / digital read-out".

A page-based interface: the ten modules are not crammed onto one board any more.
The ALL tab is an overview; every other tab gives one module the whole desk.

The look is hardware: brushed plates with bevels and screws, knurled dials with
a brushed-metal cap and a printed tick scale, engraved labels, and a small
digital LED window under every control.  Everything is drawn around the same
vermilion lamp as the reference.

Numbers in this file are the numbers in plugin/Source/PluginEditor.cpp.

    python3 render_v3.py     ->  v3-all.png / v3-eq.png / v3-dyn.png / v3-space.png
                                 (2880x1800 each)  +  v3-chat-*.jpg (1440x900)
"""
import math
import os

import numpy as np
from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

S = 3                                  # supersample
W, H = 1440, 900

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
PORTRAIT = os.path.join(ROOT, "assets", "brand", "portrait-2026-vermilion.png")

# ----------------------------------------------------------------- palette
BLACK     = (7, 7, 8)
PLATE_TOP = (25, 27, 31)
PLATE_BOT = (13, 14, 17)
PLATE_EDG = (46, 50, 57)
BEVEL_HI  = (66, 71, 79)
INK       = (236, 239, 242)
INK2      = (198, 204, 211)
MUT       = (140, 147, 157)
DIM       = (95, 102, 112)
FAINT     = (64, 69, 78)
ACC       = (208, 54, 28)
DEEP      = (136, 33, 17)
SOFT      = (242, 116, 90)
SCALE     = (132, 138, 147)
METAL_HI  = (206, 210, 215)
METAL_LO  = (112, 117, 124)
LED_BG    = (13, 11, 11)

FDIR = "/usr/share/fonts/truetype/dejavu/"
F_SANS  = FDIR + "DejaVuSans.ttf"
F_BOLD  = FDIR + "DejaVuSans-Bold.ttf"
F_MONO  = FDIR + "DejaVuSansMono.ttf"
F_SERIF = FDIR + "DejaVuSerif.ttf"

_fonts = {}
_rng = np.random.default_rng (7)


def font(path, size):
    key = (path, round (size, 2))
    if key not in _fonts:
        _fonts[key] = ImageFont.truetype (path, max (5, int (round (size * S))))
    return _fonts[key]


def px(v):
    return int (round (v * S))


def P(pts):
    return [ (px (x), px (y)) for x, y in pts ]


def new_board():
    return Image.new ("RGB", (W * S, H * S), BLACK)


# ----------------------------------------------------------------- text
def text(d, xy, s, size, colour=INK, anchor="mm", track=0.0, bold=False, mono=False,
         serif=False):
    f = font (F_MONO if mono else (F_SERIF if serif else (F_BOLD if bold else F_SANS)), size)
    if track <= 0.0:
        d.text ((px (xy[0]), px (xy[1])), s, font=f, fill=colour, anchor=anchor)
        return

    step = track * S
    widths = [ d.textlength (ch, font=f) for ch in s ]
    total = sum (widths) + step * (len (s) - 1)
    x = px (xy[0])
    if anchor[0] == "m":
        x -= total / 2.0
    elif anchor[0] == "r":
        x -= total
    for ch, w in zip (s, widths):
        if ch != " ":
            d.text ((x, px (xy[1])), ch, font=f, fill=colour, anchor="l" + anchor[1])
        x += w + step


def pline(d, pts, colour, width=1.0):
    d.line (P (pts), fill=colour, width=max (1, px (width)), joint="curve")


def dline(d, x0, y0, x1, y1, colour=PLATE_EDG, width=1.0):
    d.line ([ px (x0), px (y0), px (x1), px (y1) ], fill=colour, width=max (1, px (width)))


# ----------------------------------------------------------------- hardware
def brushed(img, box, radius=6, top=PLATE_TOP, bottom=PLATE_BOT, streaks=0.55):
    w, h = px (box[2] - box[0]), px (box[3] - box[1])
    if w <= 2 or h <= 2:
        return

    t = np.linspace (0.0, 1.0, h)[:, None]
    col = np.array (top, float)[None, :] * (1 - t) + np.array (bottom, float)[None, :] * t
    arr = np.repeat (col[:, None, :], w, axis=1)

    # fine horizontal brush strokes
    line = np.cumsum (_rng.normal (0, 1, (h, 1)), axis=0)
    line = line - line.mean()
    line = np.clip (line / (np.abs (line).max() + 1e-6), -1, 1) * streaks
    arr += np.repeat (line[:, :, None], w, axis=1)

    # a slow diagonal sheen, as if a lamp sits up and to the left
    xx = np.linspace (0, 1, w)[None, :]
    yy = np.linspace (0, 1, h)[:, None]
    arr += (np.clip (1.25 - (xx * 0.55 + yy * 0.75), 0, 1) * 7.0)[:, :, None]

    mask = Image.new ("L", (w, h), 0)
    ImageDraw.Draw (mask).rounded_rectangle ([0, 0, w - 1, h - 1], radius=px (radius), fill=255)
    img.paste (Image.fromarray (arr.clip (0, 255).astype (np.uint8), "RGB"),
               (px (box[0]), px (box[1])), mask)


def shadow(img, box, pad=10.0, blur=6.0, amount=0.72, drop=(1.0, 4.0)):
    x0 = max (0, px (box[0] - pad)); y0 = max (0, px (box[1] - pad))
    x1 = min (img.width, px (box[2] + pad)); y1 = min (img.height, px (box[3] + pad))
    if x1 <= x0 or y1 <= y0:
        return

    region = img.crop ((x0, y0, x1, y1))
    m = Image.new ("L", region.size, 0)
    ImageDraw.Draw (m).rounded_rectangle (
        [ px (box[0] - pad + drop[0]) - x0, px (box[1] - pad + drop[1]) - y0,
          px (box[2] + pad + drop[0]) - x0, px (box[3] + pad + drop[1]) - y0 ],
        radius=px (7), fill=255)
    m = m.filter (ImageFilter.GaussianBlur (px (blur)))

    k = (1.0 - amount * (np.asarray (m, dtype=np.float32) / 255.0))[:, :, None]
    arr = (np.asarray (region, dtype=np.float32) * k).clip (0, 255).astype (np.uint8)
    img.paste (Image.fromarray (arr, "RGB"), (x0, y0))


def screw(d, x, y, r=3.2, angle=35.0):
    d.ellipse ([ px (x - r), px (y - r), px (x + r), px (y + r) ],
               fill=(58, 62, 69), outline=(28, 31, 36), width=px (0.7))
    d.ellipse ([ px (x - r * 0.62), px (y - r * 0.62), px (x + r * 0.62), px (y + r * 0.62) ],
               fill=(74, 79, 87))
    a = math.radians (angle)
    d.line ([ px (x - math.cos (a) * r * 0.55), px (y - math.sin (a) * r * 0.55),
              px (x + math.cos (a) * r * 0.55), px (y + math.sin (a) * r * 0.55) ],
            fill=(26, 29, 34), width=max (1, px (0.9)))


def plate(img, box, title="", on=True, readout="", screws=True, radius=7):
    shadow (img, box)
    brushed (img, box, radius)

    d = ImageDraw.Draw (img)
    d.rounded_rectangle ([ px (box[0]), px (box[1]), px (box[2]), px (box[3]) ],
                         radius=px (radius), outline=PLATE_EDG, width=px (1))

    # bevel: light top edge, dark bottom edge
    d.line ([ px (box[0] + radius + 3), px (box[1] + 1), px (box[2] - radius - 3), px (box[1] + 1) ],
            fill=BEVEL_HI, width=px (1))
    d.line ([ px (box[0] + radius + 3), px (box[3] - 1), px (box[2] - radius - 3), px (box[3] - 1) ],
            fill=(6, 7, 9), width=px (1))

    if screws:
        screw (d, box[0] + 11, box[1] + 11, 3.0, 30)
        screw (d, box[2] - 11, box[1] + 11, 3.0, 120)
        screw (d, box[0] + 11, box[3] - 11, 3.0, 75)
        screw (d, box[2] - 11, box[3] - 11, 3.0, 10)

    if title:
        text (d, (box[0] + 30, box[1] + 20), title.upper(), 10.2, INK if on else MUT,
              track=2.6, anchor="lm")
        dline (d, box[0] + 26, box[1] + 36, box[2] - 26, box[1] + 36, (10, 11, 13), 1.0)
        dline (d, box[0] + 26, box[1] + 37, box[2] - 26, box[1] + 37, (44, 48, 55), 1.0)

    if readout:
        text (d, (box[2] - 30, box[1] + 20), readout, 7.6, DIM, track=1.4, anchor="rm")


def toggle(img, x, y, on, w=52.0, h=20.0, label=True):
    """A physical rocker: recessed slot, metal lever, and a status lamp."""
    d = ImageDraw.Draw (img)
    d.rounded_rectangle ([ px (x), px (y), px (x + w), px (y + h) ], radius=px (h / 2),
                         fill=(10, 11, 13), outline=(52, 57, 64), width=px (1))
    d.line ([ px (x + 3), px (y + 1.5), px (x + w - 3), px (y + 1.5) ], fill=(0, 0, 0), width=px (1))

    lever_w = w * 0.52
    lx = x + (w - lever_w - 3) if on else x + 3
    ly = y + 2.4
    lh = h - 4.8

    d.rounded_rectangle ([ px (lx), px (ly), px (lx + lever_w), px (ly + lh) ],
                         radius=px (lh / 2), fill=(46, 50, 57), outline=(24, 27, 31), width=px (0.8))
    d.rounded_rectangle ([ px (lx + 1), px (ly + 1), px (lx + lever_w - 1), px (ly + lh * 0.48) ],
                         radius=px (lh / 2), fill=(62, 67, 75))

    lamp_x = x + w + 9.5
    lamp_y = y + h / 2
    if on:
        d.ellipse ([ px (lamp_x - 4.6), px (lamp_y - 4.6), px (lamp_x + 4.6), px (lamp_y + 4.6) ],
                   fill=DEEP)
        d.ellipse ([ px (lamp_x - 2.6), px (lamp_y - 2.6), px (lamp_x + 2.6), px (lamp_y + 2.6) ],
                   fill=ACC)
        d.ellipse ([ px (lamp_x - 1.1), px (lamp_y - 1.9), px (lamp_x + 0.7), px (lamp_y - 0.1) ],
                   fill=SOFT)
    else:
        d.ellipse ([ px (lamp_x - 2.8), px (lamp_y - 2.8), px (lamp_x + 2.8), px (lamp_y + 2.8) ],
                   fill=(18, 19, 22), outline=(58, 63, 71), width=px (0.9))

    if label:
        text (d, (lamp_x + 8, lamp_y + 0.5), "ON" if on else "OFF", 7.0,
              SOFT if on else DIM, track=1.3, anchor="lm")


def led(img, box, value, caption="", size=9.0, align="right"):
    """A recessed digital window: caption outside, value inside in mono."""
    d = ImageDraw.Draw (img)
    d.rounded_rectangle ([ px (box[0]), px (box[1]), px (box[2]), px (box[3]) ],
                         radius=px (2.5), fill=LED_BG, outline=(40, 44, 50), width=px (0.9))
    d.line ([ px (box[0] + 3), px (box[1] + 1.2), px (box[2] - 3), px (box[1] + 1.2) ],
            fill=(0, 0, 0), width=px (1))

    text (d, ((box[0] + box[2]) / 2.0, (box[1] + box[3]) / 2.0 + 0.4), value, size,
          SOFT, track=0.4, mono=True)

    if caption:
        text (d, ((box[0] + box[2]) / 2.0, box[3] + 9.0), caption, 7.2, MUT, track=1.6)


# ----------------------------------------------------------------- the dial
def metal_cap(img, cx, cy, r):
    """Brushed gunmetal cap: soft vertical banding, one specular pool up-left."""
    size = px (r * 2)
    if size < 6:
        return

    yy = np.linspace (-1.0, 1.0, size)[:, None]
    xx = np.linspace (-1.0, 1.0, size)[None, :]

    top = np.array ((104.0, 108.0, 114.0))
    bot = np.array ((40.0, 43.0, 48.0))
    t = np.clip ((yy + 1.0) * 0.5, 0, 1)[:, :, None]
    arr = top[None, None, :] * (1 - t) + bot[None, None, :] * t
    arr = np.repeat (arr, size, axis=1)

    rad = np.sqrt (xx * xx + yy * yy)
    arr *= np.clip (1.02 - rad * 0.30, 0.42, 1.02)[:, :, None]

    spec = np.exp (-(((xx + 0.36) ** 2 + (yy + 0.46) ** 2) / 0.16))
    arr += spec[:, :, None] * 26.0

    # concentric turning marks
    rings = (np.sin (rad * r * 2.4) * 3.2)[:, :, None]
    arr += rings

    mask = Image.new ("L", (size, size), 0)
    ImageDraw.Draw (mask).ellipse ([0, 0, size - 1, size - 1], fill=255)
    img.paste (Image.fromarray (arr.clip (0, 255).astype (np.uint8), "RGB"),
               (px (cx - r), px (cy - r)), mask)


def dial(img, cx, cy, r, label="", value="", frac=0.5, ticks=True, arc=True,
         label_size=8.6, value_size=8.2):
    d = ImageDraw.Draw (img)
    a0, a1 = 135.0, 405.0

    # printed tick scale: majors only on small dials, majors + minors on big ones
    if ticks and r >= 10:
        big_dial = r >= 26
        n = 30 if big_dial else 20
        for i in range (n + 1):
            t = i / n
            ang = math.radians (a0 + (a1 - a0) * t)
            major = (i % (3 if big_dial else 2) == 0)
            if not big_dial and not major:
                continue
            r0, r1 = r + 3.4, r + (7.4 if major else 5.2)
            d.line ([ px (cx + math.cos (ang) * r0), px (cy + math.sin (ang) * r0),
                      px (cx + math.cos (ang) * r1), px (cy + math.sin (ang) * r1) ],
                    fill=(132, 138, 147) if major else (84, 90, 98),
                    width=max (1, px (0.9 if major else 0.65)))

    # value arc, just inside the ticks
    if arc and frac > 0.004:
        d.arc ([ px (cx - r - 1.4), px (cy - r - 1.4), px (cx + r + 1.4), px (cy + r + 1.4) ],
               a0, a0 + (a1 - a0) * frac, fill=ACC, width=max (1, px (1.9)))

    # knurled skirt
    d.ellipse ([ px (cx - r), px (cy - r), px (cx + r), px (cy + r) ],
               fill=(17, 18, 22), outline=(56, 61, 69), width=px (1))
    d.ellipse ([ px (cx - r + 1.6), px (cy - r + 1.6), px (cx + r - 1.6), px (cy + r - 1.6) ],
               outline=(9, 10, 12), width=px (0.8))

    nk = 72 if r > 26 else 56
    for i in range (nk):
        ang = math.radians (a0 + (a1 - a0) * (i / (nk - 1)) * 0.999)
        x0 = cx + math.cos (ang) * (r - 1.2)
        y0 = cy + math.sin (ang) * (r - 1.2)
        x1 = cx + math.cos (ang) * (r * 0.80)
        y1 = cy + math.sin (ang) * (r * 0.80)
        d.line ([ px (x0), px (y0), px (x1), px (y1) ],
                fill=(64, 69, 77) if i % 2 == 0 else (26, 28, 33), width=px (0.7))

    # cap
    cap = r * 0.72
    d.ellipse ([ px (cx - cap - 1.2), px (cy - cap - 1.2),
                 px (cx + cap + 1.2), px (cy + cap + 1.2) ], fill=(12, 13, 15))
    metal_cap (img, cx, cy, cap)

    d = ImageDraw.Draw (img)
    d.ellipse ([ px (cx - cap), px (cy - cap), px (cx + cap), px (cy + cap) ],
               outline=(38, 42, 48), width=px (0.8))

    # pointer: a dark groove with a lit tip
    ang = math.radians (a0 + (a1 - a0) * frac)
    x0, y0 = cx + math.cos (ang) * cap * 0.18, cy + math.sin (ang) * cap * 0.18
    x1, y1 = cx + math.cos (ang) * cap * 0.92, cy + math.sin (ang) * cap * 0.92
    dline (d, x0, y0, x1, y1, (22, 24, 28), max (1.6, r * 0.085))
    dline (d, x0 + 0.4, y0 - 0.4, x1, y1, (250, 250, 252), max (0.8, r * 0.045))

    # centre pin
    d.ellipse ([ px (cx - r * 0.07), px (cy - r * 0.07),
                 px (cx + r * 0.07), px (cy + r * 0.07) ], fill=(28, 31, 36))

    if label:
        text (d, (cx, cy + r + 20), label, label_size, INK2, track=1.5)
    if value:
        w = max (min (34.0, r * 1.5 + 12.0), len (value) * (value_size * 0.62) + 10.0)
        led (img, (cx - w / 2, cy + r + 25, cx + w / 2, cy + r + 41), value, size=value_size)


# ----------------------------------------------------------------- read-outs
def meter_h(img, box, label, level_db, segments=44):
    d = ImageDraw.Draw (img)
    d.rounded_rectangle ([ px (box[0]), px (box[1]), px (box[2]), px (box[3]) ],
                         radius=px (2.5), fill=(10, 11, 13), outline=(38, 42, 49), width=px (1))

    n = max (0.0, min (1.0, (level_db + 60.0) / 60.0))
    inner = (box[0] + 2, box[1] + 2, box[2] - 2, box[3] - 2)
    w = (inner[2] - inner[0]) / segments

    for i in range (segments):
        x0 = inner[0] + i * w
        lit = (i + 1) / segments <= n
        col = ACC if i < segments * 0.82 else SOFT
        if not lit:
            col = (26, 22, 22) if i < segments * 0.82 else (30, 24, 22)
        d.rectangle ([ px (x0), px (inner[1]), px (x0 + w - 1.1), px (inner[3]) ], fill=col)

    text (d, (box[0], box[1] - 9), label, 7.0, MUT, track=1.6, anchor="lm")


def vu(img, box, label, level_db):
    """A real moving-coil meter: printed scale, red zone, needle."""
    d = ImageDraw.Draw (img)
    d.rounded_rectangle ([ px (box[0]), px (box[1]), px (box[2]), px (box[3]) ],
                         radius=px (3), fill=(232, 228, 218), outline=(40, 44, 50), width=px (1.4))
    d.rounded_rectangle ([ px (box[0] + 2), px (box[1] + 2), px (box[2] - 2), px (box[3] - 2) ],
                         radius=px (2.4), fill=(223, 218, 206))

    cx = (box[0] + box[2]) / 2.0
    cy = box[3] - 6
    r = min (box[2] - box[0], box[3] - box[1]) * 0.78

    # red zone before the last fifth of the arc
    d.arc ([ px (cx - r), px (cy - r), px (cx + r), px (cy + r) ], 200, 236,
           fill=(178, 44, 26), width=px (3.2))

    for i in range (13):
        ang = math.radians (196 + 148 * (i / 12))
        major = i % 3 == 0
        r0 = r - (2.0 if major else 1.0)
        r1 = r + (6.0 if major else 3.6)
        d.line ([ px (cx + math.cos (ang) * r0), px (cy + math.sin (ang) * r0),
                  px (cx + math.cos (ang) * r1), px (cy + math.sin (ang) * r1) ],
                fill=(44, 42, 40), width=px (1.1 if major else 0.7))
        if major:
            text (d, (cx + math.cos (ang) * (r - 11), cy + math.sin (ang) * (r - 11)),
                  ["-20", "-10", "-5", "-3", "0", "+3"][i // 2] if i // 2 < 6 else "0",
                  5.6, (52, 50, 48), track=0.1, mono=True)

    n = max (0.0, min (1.0, (level_db + 24.0) / 27.0))
    ang = math.radians (196 + 148 * n)
    dline (d, cx, cy, cx + math.cos (ang) * (r - 6), cy + math.sin (ang) * (r - 6),
           (150, 32, 18), 1.5)
    d.ellipse ([ px (cx - 3), px (cy - 3), px (cx + 3), px (cy + 3) ], fill=(52, 50, 48))

    text (d, (cx, box[1] + 8), label, 5.8, (70, 66, 62), track=2.4)


# ----------------------------------------------------------------- artwork
def art_eq(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    mid = a[1] + h / 2.0

    for i in range (1, 8):
        x = a[0] + w * i / 8.0
        dline (d, x, a[1], x, a[3], (33, 36, 42), 1.0)
    for i in range (1, 4):
        y = a[1] + h * i / 4.0
        dline (d, a[0], y, a[2], y, (33, 36, 42), 1.0)
    dline (d, a[0], mid, a[2], mid, (52, 57, 64), 1.0)

    pts = []
    for i in range (140):
        t = i / 139.0
        f = 20.0 * (1000.0 ** t)
        bell = lambda fc, oct: math.exp (-0.5 * (math.log2 (f / fc) / oct) ** 2)
        db = (3.0 * bell (65, 1.15) - 2.4 * bell (152, 0.85) - 3.5 * bell (3500, 0.95)
              - 4.0 * bell (9000, 1.5) - 1.0 * (t - 0.5) * 6.0)
        if f < 32:
            db -= 10.0 * math.log2 (32.0 / max (20.0, f))
        pts.append ((a[0] + t * w, mid - (db / 14.0) * (h * 0.44)))

    pline (d, pts, SOFT, 1.8)
    for (bx, by) in (pts[12], pts[34], pts[74], pts[108]):
        d.ellipse ([ px (bx - 3.4), px (by - 3.4), px (bx + 3.4), px (by + 3.4) ],
                   fill=(246, 240, 232), outline=ACC, width=px (1.1))


def art_comp(d, a, gr=-6.8, img=None):
    w, h = a[2] - a[0], a[3] - a[1]
    left = (a[0], a[1], a[0] + w * 0.62, a[3])

    d.rounded_rectangle ([ px (left[0]), px (left[1]), px (left[2]), px (left[3]) ],
                         radius=px (4), fill=(10, 11, 13), outline=(38, 42, 49), width=px (1))
    for i in range (1, 4):
        dline (d, left[0], left[1] + (left[3] - left[1]) * i / 4.0, left[2],
               left[1] + (left[3] - left[1]) * i / 4.0, (26, 29, 34), 1.0)
    for i in range (1, 4):
        dline (d, left[0] + (left[2] - left[0]) * i / 4.0, left[1],
               left[0] + (left[2] - left[0]) * i / 4.0, left[3], (26, 29, 34), 1.0)

    pts = [ (left[0] + 5 + (left[2] - left[0] - 10) * (i / 49.0) ** 0.60,
             left[3] - 5 - (left[3] - left[1] - 10) * (i / 49.0)) for i in range (50) ]
    pline (d, pts, ACC, 1.7)

    gx = a[0] + w * 0.70
    text (d, (gx, a[1] + 3), "GAIN REDUCTION", 6.4, DIM, track=1.3, anchor="lm")
    bar = (gx, a[1] + 12, a[2], a[3] - 2)
    d.rounded_rectangle ([ px (bar[0]), px (bar[1]), px (bar[2]), px (bar[3]) ],
                         radius=px (3), fill=(10, 11, 13), outline=(38, 42, 49), width=px (1))
    amount = max (0.0, min (1.0, -gr / 16.0))
    if amount > 0.01:
        d.rounded_rectangle ([ px (bar[0] + 2), px (bar[1] + 2),
                               px (bar[0] + (bar[2] - bar[0]) * amount), px (bar[3] - 2) ],
                             radius=px (2), fill=ACC)
    if img is not None:
        led (img, (bar[0], bar[3] + 4, bar[0] + 44, bar[3] + 18), f"{gr:.1f}", size=7.4)


def art_sat(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    d.rounded_rectangle ([ px (a[0]), px (a[1]), px (a[2]), px (a[3]) ],
                         radius=px (4), fill=(10, 11, 13), outline=(38, 42, 49), width=px (1))
    mid = (a[1] + a[3]) / 2.0
    pts = [ (a[0] + 6 + (i / 119.0) * (w - 12),
             mid - (h * 0.40) * math.tanh (math.sin (i / 119.0 * 6 * math.pi) * 3.0))
            for i in range (120) ]
    pline (d, pts, SOFT, 1.6)


def art_decay(d, a, rate=2.6):
    w, h = a[2] - a[0], a[3] - a[1]
    d.rounded_rectangle ([ px (a[0]), px (a[1]), px (a[2]), px (a[3]) ],
                         radius=px (4), fill=(10, 11, 13), outline=(38, 42, 49), width=px (1))
    pts = [ (a[0] + 5 + (i / 89.0) * (w - 10),
             a[3] - 5 - math.exp (-(i / 89.0) * rate) * (h - 12)) for i in range (90) ]
    pline (d, pts, SOFT, 1.7)


def art_delay(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    d.rounded_rectangle ([ px (a[0]), px (a[1]), px (a[2]), px (a[3]) ],
                         radius=px (4), fill=(10, 11, 13), outline=(38, 42, 49), width=px (1))
    for c in range (11):
        amp = max (0.07, 0.72 ** c)
        x = a[0] + 8 + (w - 16) * c / 10.0
        for r in range (2):
            y = a[1] + (a[3] - a[1]) * (0.34 + 0.32 * r)
            rad = 1.6 + 1.9 * amp
            d.ellipse ([ px (x - rad), px (y - rad), px (x + rad), px (y + rad) ],
                       fill=tuple (int (v * min (1.0, amp * 1.5)) for v in SOFT))


def art_lfo(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    d.rounded_rectangle ([ px (a[0]), px (a[1]), px (a[2]), px (a[3]) ],
                         radius=px (4), fill=(10, 11, 13), outline=(38, 42, 49), width=px (1))
    pts = [ (a[0] + 5 + (i / 99.0) * (w - 10),
             a[1] + h / 2 - (h * 0.32) * math.sin (i / 99.0 * 4 * math.pi)) for i in range (100) ]
    pline (d, pts, SOFT, 1.6)


def art_fan(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    d.rounded_rectangle ([ px (a[0]), px (a[1]), px (a[2]), px (a[3]) ],
                         radius=px (4), fill=(10, 11, 13), outline=(38, 42, 49), width=px (1))
    cx, cy = (a[0] + a[2]) / 2.0, a[3] - 3
    r = min (w * 0.42, (h - 4) * 0.95)
    d.arc ([ px (cx - r), px (cy - r), px (cx + r), px (cy + r) ], 180, 360,
           fill=(58, 62, 70), width=px (1.2))
    for i in range (-6, 7):
        ang = math.radians (270 + 38 * i / 6.0)
        lit = abs (i) <= 2
        dline (d, cx, cy, cx + math.cos (ang) * r, cy + math.sin (ang) * r,
               ACC if lit else (58, 56, 60), 1.2 if lit else 0.9)


def art_transient(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    d.rounded_rectangle ([ px (a[0]), px (a[1]), px (a[2]), px (a[3]) ],
                         radius=px (4), fill=(10, 11, 13), outline=(38, 42, 49), width=px (1))
    pts = [ (a[0] + 5 + (i / 89.0) * (w - 10),
             a[3] - 5 - (math.exp (-(i / 89.0) * 5.0) * 0.80
                         + (1 - math.exp (-(i / 89.0) * 12)) * 0.30) * (h - 12))
           for i in range (90) ]
    pline (d, pts, SOFT, 1.6)


def art_spectrum(img, box):
    """A screen, not a card: fine grid, a lit curve, a soft glow under it."""
    d = ImageDraw.Draw (img)
    d.rounded_rectangle ([ px (box[0]), px (box[1]), px (box[2]), px (box[3]) ],
                         radius=px (3), fill=(8, 9, 11), outline=(38, 42, 49), width=px (1))

    inner = (box[0] + 30, box[1] + 12, box[2] - 12, box[3] - 20)
    for i in range (1, 7):
        x = inner[0] + (inner[2] - inner[0]) * i / 7.0
        dline (d, x, inner[1], x, inner[3], (24, 26, 31), 1.0)
    for db in (-12, -24, -36, -48, -60):
        y = inner[3] - (db + 60) / 60.0 * (inner[3] - inner[1])
        dline (d, inner[0], y, inner[2], y, (24, 26, 31), 1.0)
        text (d, (box[0] + 6, y), str (db).replace ("-", "\u2212"), 6.0, (74, 80, 90),
              track=0.2, anchor="lm", mono=True)

    anchors = [ (20, -44), (35, -30), (60, -18), (110, -14), (200, -18), (400, -24),
                (700, -27), (1200, -31), (2500, -35), (5000, -40), (9000, -46),
                (14000, -52), (20000, -60) ]

    def adb(f):
        lf = math.log (f)
        for (f0, d0), (f1, d1) in zip (anchors, anchors[1:]):
            if f0 <= f <= f1:
                t = (lf - math.log (f0)) / (math.log (f1) - math.log (f0))
                return d0 + (d1 - d0) * t
        return anchors[-1][1]

    pts = []
    for i in range (220):
        t = i / 219.0
        f = 20.0 * 1000.0 ** t
        v = adb (f) + 0.9 * math.sin (t * 92.0) + 0.7 * math.sin (t * 27.0 + 1.2)
        pts.append ((inner[0] + t * (inner[2] - inner[0]),
                     inner[3] - (max (-58.0, min (-7.0, v)) + 60.0) / 60.0
                     * (inner[3] - inner[1])))

    pline (d, pts, SOFT, 1.7)

    for f, lab in ((20, "20"), (100, "100"), (500, "500"), (1000, "1k"), (5000, "5k"),
                   (10000, "10k"), (20000, "20k")):
        t = math.log (max (20.0, f) / 20.0) / math.log (1000.0)
        x = inner[0] + t * (inner[2] - inner[0])
        text (d, (x, box[3] - 10), lab, 6.0, (74, 80, 90), track=0.2)


# ----------------------------------------------------------------- frames
def header(img, page="ALL"):
    d = ImageDraw.Draw (img)
    brushed (img, (0.0, 0.0, W, 62.0), 0, (19, 21, 24), (12, 13, 16), 0.35)
    d = ImageDraw.Draw (img)
    dline (d, 0, 62, W, 62, (2, 2, 3), 1.2)
    dline (d, 0, 61, W, 61, (52, 57, 64), 0.9)

    # monogram
    x, y, h = 22.0, 17.0, 27.0
    w = h * 0.40
    mono = []
    for k in range (2):
        ox = x + k * (w + h * 0.16)
        mono += [ (ox, y + h), (ox, y), (ox + w, y + h * 0.60), (ox + w, y + h) ]
    pline (d, mono, INK, 1.8)
    dline (d, x + w + h * 0.08, y + h * 0.30, x + w + h * 0.08, y + h, ACC, 1.5)

    dline (d, 82, 12, 82, 50, (48, 52, 59), 1.0)
    text (d, (94, 24), "MH SIGNATURE", 13.0, INK, track=4.0, anchor="lm")
    text (d, (95, 42), "BY MILAD HASHEMI", 6.8, DIM, track=3.2, anchor="lm")

    tabs = ["ALL", "EQ", "DYN", "COLOR", "SPACE", "MOD", "IMAGE"]
    widths = [46, 40, 44, 58, 62, 44, 62]
    gap = 22.0
    total = sum (widths) + gap * (len (tabs) - 1)
    x = 776 - total / 2.0
    for label, wd in zip (tabs, widths):
        on = label == page
        text (d, (x + wd / 2, 28), label, 8.6, INK if on else MUT, track=2.2)
        if on:
            dline (d, x + 2, 44, x + wd - 2, 44, ACC, 1.8)
            d.ellipse ([ px (x + wd / 2 - 1.6), px (48.5), px (x + wd / 2 + 1.6), px (51.7) ],
                       fill=SOFT)
        x += wd + gap

    # preset window
    box = (1120, 10, 1352, 52)
    d.rounded_rectangle ([ px (box[0]), px (box[1]), px (box[2]), px (box[3]) ],
                         radius=px (3), fill=LED_BG, outline=(46, 50, 57), width=px (1))
    text (d, (box[0] + 10, box[1] + 12), "PRESET", 6.2, DIM, track=2.0, anchor="lm")
    text (d, (box[0] + 10, box[1] + 29), "001  MODERN CLARITY", 8.4, SOFT, track=0.8,
          anchor="lm", mono=True)
    pline (d, [ (box[2] - 18, box[1] + 18), (box[2] - 13, box[1] + 24),
                (box[2] - 8, box[1] + 18) ], MUT, 1.2)

    text (d, (1428, 20), "SOUND WITH", 6.4, DIM, track=2.4, anchor="rm")
    text (d, (1428, 34), "INTENTION", 6.4, DIM, track=2.4, anchor="rm")


def footer(img):
    d = ImageDraw.Draw (img)
    brushed (img, (0.0, 862.0, W, H), 0, (14, 15, 18), (9, 10, 12), 0.3)
    d = ImageDraw.Draw (img)
    dline (d, 0, 862, W, 862, (46, 50, 57), 1.0)

    text (d, (212, 880), "VERSION 1.0.0", 6.8, FAINT, track=2.6, anchor="lm")
    text (d, (822, 880), "AN ALL-IN-ONE MIXING ENVIRONMENT", 8.6, (176, 183, 192),
          track=3.2, anchor="mm")
    text (d, (1428, 880), "DESIGNED BY MILAD HASHEMI", 6.8, FAINT, track=2.6, anchor="rm")


def portrait_rail(img, box=(8, 66, 200, 858)):
    tw, th = px (box[2] - box[0]), px (box[3] - box[1])
    aspect = tw / float (th)

    src = Image.open (PORTRAIT).convert ("RGB")
    sw, sh = src.size
    ch = int (sh * 0.955)
    cw = int (ch * aspect)
    cx = int (sw * 0.555)
    x0 = max (0, min (sw - cw, cx - cw // 2))
    face = src.crop ((x0, int (sh * 0.015), x0 + cw, int (sh * 0.015) + ch)).resize (
        (tw, th), Image.LANCZOS)

    arr = np.asarray (face).astype (np.float32)
    yy = np.linspace (0.0, 1.0, th)[:, None, None]
    xx = np.linspace (0.0, 1.0, tw)[None, :, None]

    bottom = np.clip ((yy - 0.66) / 0.34, 0, 1) ** 1.4
    top = np.clip ((0.07 - yy) / 0.07, 0, 1) ** 1.2
    right = np.clip ((xx - 0.86) / 0.14, 0, 1) ** 1.5
    veil = np.clip (bottom * 0.96 + top * 0.65 + right * 0.55, 0, 1)
    arr = arr * (1 - veil) + np.array (BLACK, np.float32) * veil

    img.paste (Image.fromarray (arr.clip (0, 255).astype (np.uint8)), (px (box[0]), px (box[1])))

    d = ImageDraw.Draw (img)
    ccx = (box[0] + box[2]) / 2.0

    # signature
    f = font (F_SERIF, 20.0)
    tmp = Image.new ("RGBA", (px (176), px (40)), (0, 0, 0, 0))
    ImageDraw.Draw (tmp).text ((px (92), px (20)), "Milad Hashemi", font=f,
                               fill=(247, 241, 235, 255), anchor="mm")
    tmp = tmp.rotate (-7, resample=Image.BICUBIC, expand=False)
    img.paste (tmp, (px (ccx) - tmp.width // 2, px (624) - tmp.height // 2), tmp)

    d = ImageDraw.Draw (img)
    pline (d, [ (box[0] + 44, 656), (box[0] + 96, 652), (box[0] + 156, 658) ], ACC, 1.2)
    text (d, (ccx, 676), "MUSIC  \u00b7  PEOPLE  \u00b7  EMOTION", 6.2, (203, 195, 189),
          track=1.8)
    text (d, (ccx, 692), "ALWAYS", 6.2, (203, 195, 189), track=1.8)

    text (d, (ccx, 796), "VERSION 1.0.0", 6.2, FAINT, track=2.6)
    text (d, (ccx, 826), "MH SIGNATURE", 6.0, (128, 134, 143), track=3.6)

    dline (d, box[2] + 0.5, box[1], box[2] + 0.5, box[3], (34, 38, 45), 1.0)


def chain_column(img, box, active=()):
    """Right rail: the engine, the ten modules of the chain, and the utility."""
    plate (img, box, "", screws=True)
    d = ImageDraw.Draw (img)

    text (d, (box[0] + 20, box[1] + 22), "SIGNAL CHAIN", 8.0, INK2, track=2.8, anchor="lm")
    dline (d, box[0] + 18, box[1] + 34, box[2] - 18, box[1] + 34, (10, 11, 13), 1.0)

    rows = [ ("ENGINE", "100%", True), ("EQ", "ON", True), ("DYNAMICS", "ON", True),
             ("LIMITER", "ON", True), ("COLOUR", "ON", True), ("VOCAL", "ON", True),
             ("REVERB", "ON", True), ("DELAY", "ON", True), ("MODULATION", "ON", True),
             ("TRANSIENT", "ON", True), ("IMAGER", "ON", True), ("OUTPUT", "\u22121.0 dB", True) ]

    y = box[1] + 50
    for i, (name, val, on) in enumerate (rows):
        hot = name in active
        if i % 2 == 0:
            d.rounded_rectangle ([ px (box[0] + 14), px (y - 9), px (box[2] - 14), px (y + 9) ],
                                 radius=px (2), fill=(20, 22, 26) if not hot else (28, 22, 20))

        lx, ly = box[0] + 24, y
        if on:
            d.ellipse ([ px (lx - 3.6), px (ly - 3.6), px (lx + 3.6), px (ly + 3.6) ],
                       fill=ACC if hot else (150, 44, 26))
            d.ellipse ([ px (lx - 1.5), px (ly - 1.5), px (lx + 1.5), px (ly + 1.5) ],
                       fill=SOFT if hot else (196, 96, 76))
        else:
            d.ellipse ([ px (lx - 3.2), px (ly - 3.2), px (lx + 3.2), px (ly + 3.2) ],
                       fill=(18, 19, 22), outline=(62, 67, 75), width=px (0.9))

        text (d, (box[0] + 38, y), name, 6.8, INK if hot else INK2, track=1.6, anchor="lm")
        text (d, (box[2] - 22, y), val, 6.4, SOFT if hot else DIM, track=0.6, anchor="rm",
              mono=True)
        y += 26

    # utility at the bottom of the rail
    uy = box[3] - 96
    dline (d, box[0] + 18, uy - 12, box[2] - 18, uy - 12, (10, 11, 13), 1.0)
    text (d, (box[0] + 20, uy + 2), "UTILITY / I/O", 7.0, INK2, track=2.2, anchor="lm")
    dial (img, box[0] + 46, uy + 34, 15.0, "", "", 0.43, ticks=False, value_size=7.2)
    dial (img, box[0] + 100, uy + 34, 15.0, "", "", 0.48, ticks=False, value_size=7.2)
    d = ImageDraw.Draw (img)
    text (d, (box[0] + 46, uy + 56), "IN", 6.0, MUT, track=1.4)
    text (d, (box[0] + 100, uy + 56), "OUT", 6.0, MUT, track=1.4)
    toggle (img, box[0] + 148, uy + 24, False, 46, 18, label=False)
    toggle (img, box[0] + 148, uy + 46, False, 46, 18, label=False)
    d = ImageDraw.Draw (img)
    text (d, (box[0] + 202, uy + 33), "MONO", 6.0, DIM, track=1.4, anchor="lm")
    text (d, (box[0] + 202, uy + 55), "BYPASS", 6.0, DIM, track=1.4, anchor="lm")


# ----------------------------------------------------------------- module data
D = {
    "engine":  [ ("INTENSITY", "100", 1.00), ("CHARACTER", "62", 0.62),
                 ("DENSITY", "48", 0.48), ("SPACE", "70", 0.70), ("WIDTH", "56", 0.56) ],
    "eq":      [ ("HPF", "32 Hz", 0.07), ("LOW", "+3.0", 0.58), ("LO MID", "\u22122.4", 0.43),
                 ("HI MID", "\u22123.5", 0.40), ("HIGH", "\u22124.0", 0.39) ],
    "dyn":     [ ("THRESH", "\u221218.0", 0.70), ("RATIO", "4.0:1", 0.16),
                 ("ATTACK", "10 ms", 0.09), ("RELEASE", "100 ms", 0.09),
                 ("MAKEUP", "+3.2", 0.13), ("CEILING", "\u22120.3", 0.96) ],
    "colour":  [ ("DRIVE", "4.5", 0.19), ("TONE", "37%", 0.37), ("MIX", "58%", 0.58) ],
    "vocal":   [ ("HPF", "95 Hz", 0.26), ("BODY", "+2.0", 0.58), ("DE-ESS", "6.5 k", 0.42),
                 ("PRES", "\u22121.5", 0.44), ("AIR", "\u22126.0", 0.33) ],
    "reverb":  [ ("DECAY", "3.4 s", 0.27), ("PREDLY", "26 ms", 0.13), ("DAMP", "70%", 0.70),
                 ("MIX", "24%", 0.24) ],
    "delay":   [ ("TIME", "375 ms", 0.18), ("FDBK", "36%", 0.38), ("FILTER", "7.2 k", 0.33),
                 ("MIX", "18%", 0.18) ],
    "mod":     [ ("RATE", "0.30", 0.05), ("DEPTH", "28%", 0.28), ("MIX", "16%", 0.16) ],
    "trns":    [ ("ATTACK", "+12", 0.56), ("SUSTAIN", "\u22128", 0.46), ("SPLIT", "180 Hz", 0.07) ],
    "img":     [ ("WIDTH", "132%", 0.66), ("BAL", "0", 0.50), ("MONO", "92 Hz", 0.26) ],
    "imgband": [ ("LOW", "0%", 0.02), ("MID", "55%", 0.55), ("HIGH", "85%", 0.85),
                 ("AIR", "120%", 0.60) ],
}

RAIL = (1144, 70, 1432, 848)


# ----------------------------------------------------------------- dial blocks
def dial_block(img, box, key, rows=1, max_r=24.0, label_size=7.6, value_size=7.0):
    """Fills a rectangle with a module's dials.  The block height is derived from
    the radius, and the radius from the space, so a dial can never grow into its
    own labels or past the plate."""
    items = D[key]
    n = len(items)
    per_row = int(math.ceil(n / float(rows)))

    slot_w = (box[2] - box[0]) / per_row
    slot_h = (box[3] - box[1]) / rows

    r = min((slot_w - 26) / 2.0, (slot_h - 12 - 44) / 2.0, max_r)
    r = max(8.0, r)

    block_h = 2 * r + 44

    for i, (lab, val, frac) in enumerate(items):
        ri, ci = i // per_row, i % per_row
        cx = box[0] + slot_w * (ci + 0.5)
        cy = box[1] + slot_h * ri + (slot_h - block_h) / 2.0 + r

        dial(img, cx, cy, r, lab, val, frac,
             label_size=min(label_size, r * 0.44), value_size=min(value_size, r * 0.40))
    return r


def module_plate(img, box, title, key, readout="", on=True, rows=1, art=None,
                 art_h=0.0, chip=None, sublabels=None, show_toggle=True, max_r=22.0,
                 art_img=None):
    plate(img, box, "", screws=True)
    d = ImageDraw.Draw(img)

    text(d, (box[0] + 30, box[1] + 20), title.upper(), 9.4, INK if on else MUT,
         track=2.4, anchor="lm")

    if show_toggle:
        toggle(img, box[2] - 30 - 72, box[1] + 10, on, 44, 18)
        d = ImageDraw.Draw(img)

    if readout:
        right = (box[2] - 30 - 86) if show_toggle else (box[2] - 30)
        text(d, (right, box[1] + 20), readout, 6.8, DIM, track=1.2, anchor="rm")

    dline(d, box[0] + 26, box[1] + 36, box[2] - 26, box[1] + 36, (10, 11, 13), 1.0)
    dline(d, box[0] + 26, box[1] + 37, box[2] - 26, box[1] + 37, (44, 48, 55), 0.9)

    top = box[1] + 46

    if chip is not None:
        cw = min(box[2] - box[0] - 52, 132)
        d.rounded_rectangle([px(box[0] + 26), px(top), px(box[0] + 26 + cw), px(top + 18)],
                            radius=px(2), fill=LED_BG, outline=(44, 48, 55), width=px(1))
        text(d, (box[0] + 33, top + 9), chip[0], 6.0, DIM, track=1.5, anchor="lm")
        text(d, (box[0] + 26 + cw - 7, top + 9), chip[1], 6.6, SOFT, track=0.8,
             anchor="rm", mono=True)
        top += 26

    if sublabels:
        half = (box[3] - top - 8) / 2.0
        for i, (lab, key2) in enumerate(sublabels):
            y = top + half * i
            d = ImageDraw.Draw(img)
            text(d, (box[0] + 30, y + 8), lab, 6.2, DIM, track=2.2, anchor="lm")
            dial_block(img, (box[0] + 16, y + 16, box[2] - 16, y + half - 2), key2,
                       max_r=min(max_r, 17.0))
            if i == 0:
                dline(d, box[0] + 26, y + half - 2, box[2] - 26, y + half - 2,
                      (12, 13, 16), 1.0)
        return

    if art_img is not None and art_h > 0:
        art_img(img, (box[0] + 26, top, box[2] - 26, top + art_h))
        top += art_h + 12
    elif art is not None and art_h > 0:
        art(d, (box[0] + 26, top, box[2] - 26, top + art_h))
        top += art_h + 12

    dial_block(img, (box[0] + 16, top, box[2] - 16, box[3] - 8), key, rows=rows,
               max_r=max_r)


def analyser_plate(img, box, stats=True):
    plate(img, box, "", screws=True)
    d = ImageDraw.Draw(img)
    text(d, (box[0] + 30, box[1] + 20), "ANALYSER", 8.8, INK, track=2.8, anchor="lm")

    for i, lab in enumerate(("SPECTRUM", "WAVE", "LOUDNESS")):
        text(d, (box[0] + 122 + i * 62, box[1] + 20), lab, 6.2, INK2 if i == 0 else DIM,
             track=1.5, anchor="lm")
        if i == 0:
            dline(d, box[0] + 120, box[1] + 27, box[0] + 120 + 52, box[1] + 27, ACC, 1.2)

    bottom = box[3] - (54 if stats else 14)
    art_spectrum(img, (box[0] + 24, box[1] + 34, box[2] - 24, bottom))

    if stats:
        d = ImageDraw.Draw(img)
        for i, (cap, val) in enumerate((("LUFS", "\u22127.6"), ("PEAK", "\u22120.3"),
                                        ("CREST", "12.1"), ("WIDTH", "0.71"))):
            x = box[0] + 28 + i * 152
            text(d, (x, box[3] - 28), cap, 6.0, DIM, track=1.6, anchor="lm")
            led(img, (x + 40, box[3] - 40, x + 108, box[3] - 22), val, size=8.2)
            d = ImageDraw.Draw(img)


def art_comp_full(img, box):
    """Transfer curve on the left, a scaled gain-reduction meter on the right."""
    d = ImageDraw.Draw(img)
    w = box[2] - box[0]

    left = (box[0], box[1], box[0] + w * 0.66, box[3])
    d.rounded_rectangle([px(left[0]), px(left[1]), px(left[2]), px(left[3])], radius=px(3),
                        fill=(10, 11, 13), outline=(38, 42, 49), width=px(1))
    for i in range(1, 5):
        dline(d, left[0] + (left[2] - left[0]) * i / 5.0, left[1],
              left[0] + (left[2] - left[0]) * i / 5.0, left[3], (24, 27, 32), 1.0)
        dline(d, left[0], left[1] + (left[3] - left[1]) * i / 5.0, left[2],
              left[1] + (left[3] - left[1]) * i / 5.0, (24, 27, 32), 1.0)

    pts = [(left[0] + 8 + (left[2] - left[0] - 16) * (i / 59.0) ** 0.62,
            left[3] - 8 - (left[3] - left[1] - 16) * (i / 59.0)) for i in range(60)]
    pline(d, pts, ACC, 1.9)

    text(d, (left[0] + 8, left[1] + 9), "OUTPUT dB", 5.6, FAINT, track=1.2, anchor="lm")
    text(d, (left[2] - 8, left[3] - 9), "INPUT dB", 5.6, FAINT, track=1.2, anchor="rm")

    mbox = (box[0] + w * 0.72, box[1], box[2], box[3])
    d.rounded_rectangle([px(mbox[0]), px(mbox[1]), px(mbox[2]), px(mbox[3])], radius=px(3),
                        fill=(10, 11, 13), outline=(38, 42, 49), width=px(1))

    bar = (mbox[0] + 26, mbox[1] + 26, mbox[0] + 40, mbox[3] - 26)
    d.rounded_rectangle([px(bar[0]), px(bar[1]), px(bar[2]), px(bar[3])], radius=px(2.5),
                        fill=(8, 9, 11), outline=(34, 38, 44), width=px(1))

    for i in range(7):
        y = bar[3] - (bar[3] - bar[1]) * i / 6.0
        dline(d, bar[2] + 4, y, bar[2] + 9, y, (62, 67, 75), 1.0)
        text(d, (bar[2] + 13, y), str(-i * 3), 5.6, DIM, track=0.2, anchor="lm", mono=True)

    gr = -6.8
    top = bar[1] + (bar[3] - bar[1]) * (1 - min(1.0, -gr / 18.0))
    d.rounded_rectangle([px(bar[0] + 1), px(top), px(bar[2] - 1), px(bar[3] - 1)],
                        radius=px(2), fill=ACC)

    text(d, (mbox[0] + 14, mbox[1] + 15), "GAIN REDUCTION", 5.6, DIM, track=1.1, anchor="lm")
    led(img, (mbox[0] + 70, mbox[3] - 32, mbox[2] - 14, mbox[3] - 14), f"{gr:.1f}", size=7.4)


# ----------------------------------------------------------------- rails
def chain_column(img, box, active=()):
    plate(img, box, "", screws=True)
    d = ImageDraw.Draw(img)

    text(d, (box[0] + 20, box[1] + 22), "SIGNAL CHAIN", 7.6, INK2, track=2.6, anchor="lm")
    dline(d, box[0] + 18, box[1] + 34, box[2] - 18, box[1] + 34, (10, 11, 13), 1.0)

    rows = [("ENGINE", "100%"), ("EQ", "ON"), ("DYNAMICS", "ON"), ("LIMITER", "ON"),
            ("COLOUR", "ON"), ("VOCAL", "ON"), ("REVERB", "ON"), ("DELAY", "ON"),
            ("MODULATION", "ON"), ("TRANSIENT", "ON"), ("IMAGER", "ON")]

    y = box[1] + 52
    for name, val in rows:
        hot = name in active
        if hot:
            d.rounded_rectangle([px(box[0] + 12), px(y - 10), px(box[2] - 12), px(y + 10)],
                                radius=px(2), fill=(34, 24, 21))
            dline(d, box[0] + 12, y - 10, box[0] + 12, y + 10, ACC, 1.4)

        lx = box[0] + 26
        d.ellipse([px(lx - 3.2), px(lx - 3.2), px(lx + 3.2), px(lx + 3.2)],
                  fill=ACC if hot else (146, 44, 26))
        d.ellipse([px(lx - 1.3), px(lx - 1.3), px(lx + 1.3), px(lx + 1.3)],
                  fill=SOFT if hot else (188, 90, 70))

        text(d, (box[0] + 40, y), name, 6.4, INK if hot else INK2, track=1.4, anchor="lm")
        text(d, (box[2] - 22, y), val, 6.0, SOFT if hot else DIM, track=0.3, anchor="rm",
             mono=True)
        y += 25.0

    uy = box[3] - 118
    dline(d, box[0] + 18, uy, box[2] - 18, uy, (10, 11, 13), 1.0)
    text(d, (box[0] + 20, uy + 16), "UTILITY / I/O", 6.6, INK2, track=2.0, anchor="lm")

    dial(img, box[0] + 52, uy + 54, 16.0, "IN", "\u22123.2", 0.43,
         label_size=6.2, value_size=6.6)
    dial(img, box[0] + 116, uy + 54, 16.0, "OUT", "\u22121.0", 0.48,
         label_size=6.2, value_size=6.6)

    toggle(img, box[0] + 158, uy + 30, False, 40, 16, label=False)
    toggle(img, box[0] + 158, uy + 56, False, 40, 16, label=False)
    d = ImageDraw.Draw(img)
    text(d, (box[0] + 206, uy + 38), "MONO", 5.8, DIM, track=1.2, anchor="lm")
    text(d, (box[0] + 206, uy + 64), "BYPASS", 5.8, DIM, track=1.2, anchor="lm")


def portrait_rail(img, box=(8, 66, 200, 858)):
    tw, th = px(box[2] - box[0]), px(box[3] - box[1])
    aspect = tw / float(th)

    src = Image.open(PORTRAIT).convert("RGB")
    sw, sh = src.size
    ch = int(sh * 0.955)
    cw = int(ch * aspect)
    cx = int(sw * 0.555)
    x0 = max(0, min(sw - cw, cx - cw // 2))
    face = src.crop((x0, int(sh * 0.015), x0 + cw, int(sh * 0.015) + ch)).resize(
        (tw, th), Image.LANCZOS)

    arr = np.asarray(face).astype(np.float32)
    yy = np.linspace(0.0, 1.0, th)[:, None, None]
    xx = np.linspace(0.0, 1.0, tw)[None, :, None]

    bottom = np.clip((yy - 0.66) / 0.34, 0, 1) ** 1.4
    top = np.clip((0.07 - yy) / 0.07, 0, 1) ** 1.2
    right = np.clip((xx - 0.86) / 0.14, 0, 1) ** 1.5
    veil = np.clip(bottom * 0.96 + top * 0.65 + right * 0.55, 0, 1)
    arr = arr * (1 - veil) + np.array(BLACK, np.float32) * veil

    img.paste(Image.fromarray(arr.clip(0, 255).astype(np.uint8)), (px(box[0]), px(box[1])))

    d = ImageDraw.Draw(img)
    ccx = (box[0] + box[2]) / 2.0

    f = font(F_SERIF, 20.0)
    tmp = Image.new("RGBA", (px(176), px(40)), (0, 0, 0, 0))
    ImageDraw.Draw(tmp).text((px(88), px(20)), "Milad Hashemi", font=f,
                             fill=(247, 241, 235, 255), anchor="mm")
    tmp = tmp.rotate(-7, resample=Image.BICUBIC, expand=False)
    img.paste(tmp, (px(ccx) - tmp.width // 2, px(626) - tmp.height // 2), tmp)

    d = ImageDraw.Draw(img)
    pline(d, [(box[0] + 44, 656), (box[0] + 96, 652), (box[0] + 156, 658)], ACC, 1.2)
    text(d, (ccx, 674), "MUSIC  \u00b7  PEOPLE", 6.0, (203, 195, 189), track=1.6)
    text(d, (ccx, 688), "EMOTION  \u00b7  ALWAYS", 6.0, (203, 195, 189), track=1.6)

    text(d, (ccx, 790), "VERSION 1.0.0", 6.0, FAINT, track=2.6)
    text(d, (ccx, 820), "MH SIGNATURE", 5.8, (124, 130, 139), track=3.4)

    dline(d, box[2] + 0.5, box[1], box[2] + 0.5, box[3], (34, 38, 45), 1.0)


def frame(img, page):
    portrait_rail(img)
    header(img, page)
    footer(img)


# ----------------------------------------------------------------- pages
def art_vo(d, a):
    """De-esser: a narrow notch cut out of the top end."""
    w, h = a[2] - a[0], a[3] - a[1]
    d.rounded_rectangle([px(a[0]), px(a[1]), px(a[2]), px(a[3])], radius=px(3),
                        fill=(10, 11, 13), outline=(38, 42, 49), width=px(1))
    mid = a[1] + h * 0.55
    pts = []
    for i in range(120):
        t = i / 119.0
        notch = math.exp(-0.5 * ((t - 0.72) / 0.055) ** 2) * 0.72
        pts.append((a[0] + 6 + t * (w - 12), mid - (h * 0.26) * (1.0 - notch)
                    - (h * 0.10) * math.sin(t * 9)))
    pline(d, pts, SOFT, 1.5)


def hero_card(img, box, title, key, readout="", on=True, art=None, art_h=0.0,
              chip=None, max_r=40.0, rows=1):
    module_plate(img, box, title, key, readout, on=on, rows=rows, art=art, art_h=art_h,
                 chip=chip, max_r=max_r)


def focus_page(img, page, hero, companions, hero_img=None):
    """hero = (title, key, readout, art, art_h, chip, max_r)
       companions = [ (box, title, key, readout) x2 ]"""
    frame(img, page)

    hbox = (212, 70, 820, 560)
    title, key, readout, art, art_h, chip, max_r = hero
    module_plate(img, hbox, title, key, readout, art=art, art_h=art_h, chip=chip,
                 max_r=max_r, rows=1, art_img=hero_img)

    analyser_plate(img, (832, 70, 1132, 560))

    for box, t, k, ro, art, art_h in companions:
        module_plate(img, box, t, k, ro, max_r=19.0, art=art, art_h=art_h)

    chain_column(img, RAIL, active=(title,))


def page_all(img):
    frame(img, "ALL")

    box = (212, 70, 752, 300)
    plate(img, box, "", screws=True)
    d = ImageDraw.Draw(img)
    text(d, (box[0] + 30, box[1] + 20), "SIGNATURE ENGINE", 9.8, INK, track=3.4, anchor="lm")
    text(d, (box[2] - 30, box[1] + 20), "TURN IDEAS INTO SOUND", 6.2, DIM, track=2.2,
         anchor="rm")
    dline(d, box[0] + 26, box[1] + 36, box[2] - 26, box[1] + 36, (10, 11, 13), 1.0)

    cx = (box[0] + box[2]) / 2.0
    cy = box[1] + 128
    dial(img, cx, cy, 48.0, "", "", 1.0)
    d = ImageDraw.Draw(img)
    text(d, (cx, cy + 64), "INTENSITY", 7.6, INK2, track=2.6)
    led(img, (cx - 26, cy + 72, cx + 26, cy + 88), "100", size=8.0)

    for lab, val, frac, mx, my in (("CHARACTER", "62", 0.62, box[0] + 76, box[1] + 88),
                                   ("SPACE", "70", 0.70, box[2] - 76, box[1] + 88),
                                   ("DENSITY", "48", 0.48, box[0] + 76, box[1] + 178),
                                   ("WIDTH", "56", 0.56, box[2] - 76, box[1] + 178)):
        dial(img, mx, my, 18.0, lab, val, frac, label_size=6.6, value_size=6.2)

    analyser_plate(img, (764, 70, 1132, 300), stats=False)

    module_plate(img, (212, 312, 510, 582), "EQ", "eq", "4 BANDS", art=art_eq, art_h=54)
    module_plate(img, (522, 312, 822, 582), "DYNAMICS", "dyn", rows=2)
    module_plate(img, (834, 312, 1132, 582), "COLOUR", "colour",
                 art=art_sat, art_h=74, chip=("CHARACTER", "CONSOLE"))

    module_plate(img, (212, 594, 510, 848), "VOCAL", "vocal", "DE-ESS 6.5K",
                 art=art_vo, art_h=44)
    module_plate(img, (522, 594, 822, 848), "REVERB", "reverb", "3.4 s",
                 art=art_decay, art_h=40)
    module_plate(img, (834, 594, 1132, 848), "MOTION", "mod",
                 sublabels=(("MODULATION", "mod"), ("TRANSIENT", "trns")))

    chain_column(img, RAIL, active=("ENGINE",))


def page_eq(img):
    focus_page(img, "EQ",
               ("EQUALISER", "eq", "4 BANDS + HPF", art_eq, 170, None, 40.0),
               [((212, 572, 662, 848), "VOCAL", "vocal", "DE-ESS", art_vo, 60),
                ((674, 572, 1132, 848), "COLOUR", "colour", "CONSOLE", art_sat, 60)])


def page_dyn(img):
    focus_page(img, "DYN",
               ("DYNAMICS  /  LIMITER", "dyn", "GR \u22126.8 dB", None, 186, None, 34.0),
               [((212, 572, 662, 848), "TRANSIENT", "trns", "180 Hz", art_transient, 60),
                ((674, 572, 1132, 848), "EQ", "eq", "4 BANDS", art_eq, 60)],
               hero_img=art_comp_full)


def page_colour(img):
    focus_page(img, "COLOR",
               ("SATURATION  /  COLOUR", "colour", "CONSOLE", art_sat, 150,
                ("CHARACTER", "CONSOLE"), 34.0),
               [((212, 572, 662, 848), "VOCAL", "vocal", "DE-ESS 6.5K", art_vo, 60),
                ((674, 572, 1132, 848), "EQ", "eq", "4 BANDS", art_eq, 60)])


def page_space(img):
    frame(img, "SPACE")
    module_plate(img, (212, 70, 666, 560), "REVERB", "reverb", "3.4 s",
                 art=art_decay, art_h=190, max_r=32.0)
    module_plate(img, (678, 70, 1132, 560), "DELAY", "delay", "375 ms",
                 art=art_delay, art_h=190, max_r=32.0)
    analyser_plate(img, (212, 572, 1132, 848), stats=False)
    chain_column(img, RAIL, active=("REVERB", "DELAY"))


def page_mod(img):
    focus_page(img, "MOD",
               ("MODULATION", "mod", "0.30 Hz", art_lfo, 150, None, 34.0),
               [((212, 572, 662, 848), "TRANSIENT", "trns", "180 Hz", art_transient, 60),
                ((674, 572, 1132, 848), "REVERB", "reverb", "3.4 s", art_decay, 60)])


def page_image(img):
    focus_page(img, "IMAGE",
               ("STEREO IMAGER", "img", "M/S 92", art_fan, 170, None, 34.0),
               [((212, 572, 662, 848), "WIDTH PER BAND", "imgband", "92 Hz", None, 0),
                ((674, 572, 1132, 848), "COLOUR", "colour", "CONSOLE", art_sat, 60)])


# ----------------------------------------------------------------- finish
def finish(img, name):
    arr = np.asarray(img).astype(np.float32)

    grain = _rng.normal(0, 2.2, arr.shape[:2])[:, :, None]
    arr += grain

    h, w = arr.shape[:2]
    yy = (np.linspace(-1, 1, h)[:, None]) ** 2
    xx = (np.linspace(-1, 1, w)[None, :]) ** 2
    vig = 1.0 - 0.18 * np.clip((xx + yy) - 0.15, 0, 1.4) / 1.4
    arr *= vig[:, :, None]

    out = Image.fromarray(arr.clip(0, 255).astype(np.uint8), "RGB")
    out.save(os.path.join(HERE, f"v3-{name}.png"))
    out.resize((1440, 900), Image.LANCZOS).save(
        os.path.join(HERE, f"v3-chat-{name}.jpg"), quality=90, optimize=True)
    print("wrote", name)


if __name__ == "__main__":
    for name, fn in (("all", page_all), ("eq", page_eq), ("dyn", page_dyn),
                     ("colour", page_colour), ("space", page_space),
                     ("mod", page_mod), ("image", page_image)):
        board = new_board()
        fn(board)
        finish(board, name)
