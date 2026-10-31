#!/usr/bin/env python3
"""MH SIGNATURE — the board.

Drawn from zero against Milad's reference: black glass cards, a vermilion lamp
on every module, a full-height portrait rail, tall input/output meters, the
SIGNATURE ENGINE dial in the middle of the top band, and the ten modules of the
chain in three rows underneath.

Every rectangle, radius, type size and gap in this file is the number the JUCE
editor uses, so the render *is* the layout spec for plugin/Source/PluginEditor.cpp.

    python3 render_signature.py   ->  gui-31-signature.png   (2880x1800)
                                      chat-31-signature.jpg  (1440x900)
"""
import math
import os

import numpy as np
from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

S = 2                       # supersample factor
W, H = 1440, 900            # design space, identical to the plugin

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
PORTRAIT = os.path.join(ROOT, "assets", "brand", "portrait-2026-vermilion.png")

# --------------------------------------------------------------------- palette
NOIR    = (5, 6, 7)
CARD_HI = (16, 18, 22)
CARD_LO = (7, 8, 10)
EDGE    = (41, 45, 52)
ACC     = (208, 54, 28)
DEEP    = (143, 36, 18)
SOFT    = (242, 116, 90)
INK     = (238, 241, 245)
INK2    = (207, 213, 220)
MUT     = (142, 150, 161)
DIM     = (98, 105, 116)
FAINT   = (68, 73, 82)
TRACK   = (44, 48, 56)

FDIR = "/usr/share/fonts/truetype/dejavu/"
F_SANS  = FDIR + "DejaVuSans.ttf"
F_BOLD  = FDIR + "DejaVuSans-Bold.ttf"
F_SERIF = FDIR + "DejaVuSerif.ttf"

_fonts = {}


def font(path, size):
    key = (path, round(size, 2))
    if key not in _fonts:
        _fonts[key] = ImageFont.truetype(path, max(5, int(round(size * S))))
    return _fonts[key]


def px(v):
    return int(round(v * S))


def P(pts):
    return [(px(x), px(y)) for x, y in pts]


# ------------------------------------------------------------------ primitives
def new_board():
    return Image.new("RGB", (W * S, H * S), NOIR)


def box_mask(box, radius):
    m = Image.new("L", (px(box[2] - box[0]), px(box[3] - box[1])), 0)
    ImageDraw.Draw(m).rounded_rectangle([0, 0, m.width - 1, m.height - 1],
                                        radius=px(radius), fill=255)
    return m


def vgrad(img, box, top, bottom, radius=8):
    w, h = px(box[2] - box[0]), px(box[3] - box[1])
    if w <= 0 or h <= 0:
        return
    t = np.linspace(0.0, 1.0, h)[:, None]
    col = (np.array(top, float)[None, :] * (1 - t) + np.array(bottom, float)[None, :] * t)
    strip = np.repeat(col[:, None, :], w, axis=1).astype(np.uint8)
    img.paste(Image.fromarray(strip, "RGB"), (px(box[0]), px(box[1])),
              box_mask(box, radius))


def shadow(img, box, pad=9.0, blur=5.0, amount=0.6, drop=(1.0, 3.0)):
    """Darkens the pixels just outside a card so it sits on the desk."""
    x0 = max(0, px(box[0] - pad)); y0 = max(0, px(box[1] - pad))
    x1 = min(img.width, px(box[2] + pad)); y1 = min(img.height, px(box[3] + pad))
    if x1 <= x0 or y1 <= y0:
        return
    region = img.crop((x0, y0, x1, y1))
    m = Image.new("L", region.size, 0)
    ImageDraw.Draw(m).rounded_rectangle(
        [px(box[0] - pad + drop[0]) - x0, px(box[1] - pad + drop[1]) - y0,
         px(box[2] + pad + drop[0]) - x0, px(box[3] + pad + drop[1]) - y0],
        radius=px(radius_of(box)), fill=255)
    m = m.filter(ImageFilter.GaussianBlur(px(blur)))
    k = (1.0 - amount * (np.asarray(m, dtype=np.float32) / 255.0))[:, :, None]
    arr = (np.asarray(region, dtype=np.float32) * k).clip(0, 255).astype(np.uint8)
    img.paste(Image.fromarray(arr, "RGB"), (x0, y0))


def radius_of(box):
    return 8


def card(img, box, radius=8, glow_amount=0.0):
    """Black glass card — drop shadow, vertical skin, hairline border."""
    if glow_amount > 0.01:
        halo = Image.new("RGB", img.size, (0, 0, 0))
        ImageDraw.Draw(halo).rounded_rectangle(
            [px(box[0]), px(box[1]), px(box[2]), px(box[3])], radius=px(radius),
            fill=tuple(int(c * glow_amount) for c in DEEP))
        img.paste(ImageChops.add(img, halo.filter(ImageFilter.GaussianBlur(px(30)))), (0, 0))

    shadow(img, box, pad=9, blur=5.0, amount=0.62, drop=(1.5, 3.5))

    vgrad(img, box, CARD_HI, CARD_LO, radius)

    d = ImageDraw.Draw(img)
    d.rounded_rectangle([px(box[0]), px(box[1]), px(box[2]), px(box[3])],
                        radius=px(radius), outline=EDGE, width=px(1))
    d.line([px(box[0] + radius + 5), px(box[1] + 1), px(box[2] - radius - 5), px(box[1] + 1)],
           fill=(32, 36, 42), width=px(1))


def glow(img, painter, blur=9.0, gain=1.0):
    halo = Image.new("RGB", img.size, (0, 0, 0))
    painter(ImageDraw.Draw(halo))
    halo = halo.filter(ImageFilter.GaussianBlur(px(blur)))
    if gain != 1.0:
        halo = halo.point(lambda v: min(255, int(v * gain)))
    img.paste(ImageChops.add(img, halo), (0, 0))


def text(d, xy, s, size, colour=INK, anchor="mm", track=0.0, bold=False, serif=False):
    f = font(F_SERIF if serif else (F_BOLD if bold else F_SANS), size)
    if track <= 0.0:
        d.text((px(xy[0]), px(xy[1])), s, font=f, fill=colour, anchor=anchor)
        return

    step = track * S
    widths = [d.textlength(ch, font=f) for ch in s]
    total = sum(widths) + step * (len(s) - 1)
    x = px(xy[0])
    if anchor[0] == "m":
        x -= total / 2.0
    elif anchor[0] == "r":
        x -= total
    for ch, w in zip(s, widths):
        if ch != " ":
            d.text((x, px(xy[1])), ch, font=f, fill=colour, anchor="l" + anchor[1])
        x += w + step


def pline(d, pts, colour, width=1.0, joint="curve"):
    d.line(P(pts), fill=colour, width=max(1, px(width)), joint=joint)


def dline(d, x0, y0, x1, y1, colour=EDGE, width=1.0):
    d.line([px(x0), px(y0), px(x1), px(y1)], fill=colour, width=max(1, px(width)))


def arc(d, cx, cy, r, a0, a1, colour, width=1.5):
    d.arc([px(cx - r), px(cy - r), px(cx + r), px(cy + r)], a0, a1,
          fill=colour, width=max(1, px(width)))


def script(img, cx, cy, s, size, colour, angle=-8.0):
    """Signature: serif text set flat, then tilted into place."""
    f = font(F_SERIF, size)
    tmp = Image.new("RGBA", (int(px(size) * len(s) * 0.62), px(size * 2.4)), (0, 0, 0, 0))
    ImageDraw.Draw(tmp).text((px(4), px(size * 1.15)), s, font=f, fill=colour + (255,),
                             anchor="lm")
    tmp = tmp.rotate(angle, resample=Image.BICUBIC, expand=True)
    img.paste(tmp, (px(cx) - tmp.width // 2, px(cy) - tmp.height // 2), tmp)


# ------------------------------------------------------------------ controls
def power_switch(d, x, y, on, w=46.0, h=18.0):
    d.rounded_rectangle([px(x), px(y), px(x + w), px(y + h)], radius=px(h / 2),
                        fill=(30, 16, 13) if on else (17, 19, 23),
                        outline=ACC if on else (48, 52, 60), width=px(1))
    cx, cy = x + h / 2, y + h / 2
    r = h * 0.30
    d.ellipse([px(cx - r), px(cy - r), px(cx + r), px(cy + r)],
              fill=ACC if on else (26, 29, 35),
              outline=None if on else (66, 72, 82), width=px(1))
    text(d, (x + h + 3, cy + 0.4), "ON" if on else "OFF", 7.2,
         SOFT if on else DIM, track=1.3, anchor="lm")


def power_lamp(img, x, y, on, w=46.0, h=18.0):
    if on:
        glow(img, lambda dd: dd.rounded_rectangle(
            [px(x), px(y), px(x + w), px(y + h)], radius=px(h / 2), fill=DEEP),
            blur=6.0, gain=0.85)
    power_switch(ImageDraw.Draw(img), x, y, on, w, h)


def knob(d, cx, cy, r, label, value, frac, pointer="red", big=False,
         label_size=10.0, value_size=9.4):
    a0, a1 = 135.0, 405.0
    stroke = 3.4 if big else 2.2

    arc(d, cx, cy, r, a0, a1, TRACK, stroke)

    if frac > 0.002:
        arc(d, cx, cy, r, a0, a0 + (a1 - a0) * frac, ACC, stroke)

    body = r - (8.0 if big else 7.0)
    d.ellipse([px(cx - body), px(cy - body), px(cx + body), px(cy + body)],
              fill=(13, 14, 17))

    # brushed cap: a cool sheen over the top half
    sheen = Image.new("L", d._image.size if hasattr(d, "_image") else (px(W), px(H)), 0)

    ang = math.radians(a0 + (a1 - a0) * frac)
    x0, y0 = cx + math.cos(ang) * body * 0.28, cy + math.sin(ang) * body * 0.28
    x1, y1 = cx + math.cos(ang) * body * 0.86, cy + math.sin(ang) * body * 0.86
    dline(d, x0, y0, x1, y1, ACC if pointer == "red" else (234, 237, 242),
          2.8 if big else 2.0)

    if label:
        text(d, (cx, cy + r + 10), label, label_size, INK2, track=1.4)
    if value:
        text(d, (cx, cy + r + 22), value, value_size, SOFT, track=0.3)


def knob_face(img, cx, cy, r, big=False):
    """Glass body of a knob, drawn separately so it can glow underneath."""
    body = r - (8.0 if big else 7.0)
    layer = Image.new("RGB", img.size, (0, 0, 0))
    ld = ImageDraw.Draw(layer)
    ld.ellipse([px(cx - body), px(cy - body), px(cx + body), px(cy + body)],
               fill=(26, 29, 34))
    img.paste(ImageChops.add(img, layer.filter(ImageFilter.GaussianBlur(px(5)))), (0, 0))

    d = ImageDraw.Draw(img)
    d.ellipse([px(cx - body), px(cy - body), px(cx + body), px(cy + body)],
              fill=(14, 15, 18))
    sheen = Image.new("RGB", img.size, (0, 0, 0))
    sd = ImageDraw.Draw(sheen)
    sd.ellipse([px(cx - body * 0.92), px(cy - body * 1.02), px(cx + body * 0.92),
                px(cy + body * 0.26)], fill=(20, 22, 27))
    sd.ellipse([px(cx - body * 0.58), px(cy - body * 0.90), px(cx + body * 0.46),
                px(cy - body * 0.44)], fill=(35, 38, 45))
    img.paste(ImageChops.add(img, sheen.filter(ImageFilter.GaussianBlur(px(3.2)))), (0, 0))

    arc(ImageDraw.Draw(img), cx, cy, body - 1.0, 160, 345, (58, 63, 72), 1.1)


def draw_knob(img, cx, cy, r, label, value, frac, pointer="red", big=False,
              label_size=10.0, value_size=9.4):
    knob_face(img, cx, cy, r, big)
    d = ImageDraw.Draw(img)
    a0, a1 = 135.0, 405.0
    stroke = 3.4 if big else 2.2

    arc(d, cx, cy, r, a0, a1, TRACK, stroke)
    if frac > 0.002:
        glow(img, lambda dd: dd.arc(
            [px(cx - r), px(cy - r), px(cx + r), px(cy + r)], a0, a0 + (a1 - a0) * frac,
            fill=(190, 60, 40), width=px(stroke + 1.6)), blur=5.0, gain=0.55)
        d = ImageDraw.Draw(img)
        arc(d, cx, cy, r, a0, a0 + (a1 - a0) * frac, ACC, stroke)

    body = r - (8.0 if big else 7.0)
    ang = math.radians(a0 + (a1 - a0) * frac)
    x0, y0 = cx + math.cos(ang) * body * 0.26, cy + math.sin(ang) * body * 0.26
    x1, y1 = cx + math.cos(ang) * body * 0.88, cy + math.sin(ang) * body * 0.88

    if pointer == "red":
        glow(img, lambda dd: dd.line([px(x0), px(y0), px(x1), px(y1)],
                                     fill=ACC, width=px(2.6)), blur=4.0, gain=0.8)
        d = ImageDraw.Draw(img)
    dline(d, x0, y0, x1, y1, ACC if pointer == "red" else (236, 239, 243),
          2.8 if big else 2.0)

    if label:
        text(d, (cx, cy + r + 10), label, label_size, INK2, track=1.4)
    if value:
        text(d, (cx, cy + r + 22), value, value_size, SOFT, track=0.3)


# ------------------------------------------------------------------ geometry
R_PORTRAIT = (10, 76, 240, 852)
R_IN       = (246, 76, 300, 852)
R_OUT      = (1372, 76, 1426, 852)

R_ENGINE   = (310, 76, 806, 356)
R_SPEC     = (816, 76, 1362, 356)

R_EQ       = (310, 366, 620, 530)
R_DYN      = (630, 366, 1002, 530)
R_COLOR    = (1012, 366, 1362, 530)

R_VOCAL    = (310, 540, 640, 704)
R_REVERB   = (650, 540, 892, 704)
R_DELAY    = (902, 540, 1130, 704)
R_IMAGE    = (1140, 540, 1362, 704)

R_MOD      = (310, 714, 640, 852)
R_TRNS     = (650, 714, 982, 852)
R_IO       = (992, 714, 1362, 852)

HEADER_BAND = 66
CARD_HEAD = 32
KNOB_GAP_LABEL = 34


# ------------------------------------------------------------------ data
MODULES = {
    "EQ":      [("HPF", "32 Hz", 0.07), ("LOW", "+3.0", 0.58), ("LO MID", "-2.4", 0.43),
                ("HI MID", "-3.5", 0.40), ("HIGH", "-4.0", 0.39)],
    "DYN":     [("THRESH", "-18.0", 0.70), ("RATIO", "4.0:1", 0.16), ("ATTACK", "10 ms", 0.09),
                ("RELEASE", "100 ms", 0.09), ("MAKEUP", "+3.2", 0.13), ("CEILING", "-0.3", 0.96)],
    "COLOR":   [("DRIVE", "4.5", 0.19), ("TONE", "37%", 0.37), ("MIX", "58%", 0.58)],
    "VOCAL":   [("HPF", "95 Hz", 0.26), ("BODY", "+2.0", 0.58), ("DE-ESS", "6.5 k", 0.42),
                ("PRES", "-1.5", 0.44), ("AIR", "-6.0", 0.33)],
    "REVERB":  [("DECAY", "3.4 s", 0.27), ("PREDLY", "26 ms", 0.13), ("DAMP", "70%", 0.70),
                ("MIX", "24%", 0.24)],
    "DELAY":   [("TIME", "375 ms", 0.18), ("FDBK", "36%", 0.38), ("FILTER", "7.2 k", 0.33),
                ("MIX", "18%", 0.18)],
    "IMAGE":   [("WIDTH", "132%", 0.66), ("BAL", "0", 0.50), ("MONO", "92 Hz", 0.26)],
    "MOD":     [("RATE", "0.30", 0.05), ("DEPTH", "28%", 0.28), ("MIX", "16%", 0.16)],
    "TRNS":    [("ATTACK", "+12", 0.56), ("SUSTAIN", "-8", 0.46), ("SPLIT", "180 Hz", 0.07)],
    "IO":      [("INPUT", "-3.2", 0.43), ("OUTPUT", "-1.0", 0.48), ("MIX", "100%", 1.0)],
}


# ------------------------------------------------------------------ artwork
def art_eq(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    mid = a[1] + h / 2.0
    for i in range(1, 4):
        dline(d, a[0] + w * i / 4.0, a[1], a[0] + w * i / 4.0, a[3], (25, 28, 34), 1.0)
    dline(d, a[0], mid, a[2], mid, (30, 33, 40), 1.0)

    pts = []
    for i in range(110):
        t = i / 109.0
        f = 20.0 * (1000.0 ** t)
        bell = lambda fc, oct: math.exp(-0.5 * (math.log2(f / fc) / oct) ** 2)
        db = (3.0 * bell(65, 1.15) - 2.4 * bell(152, 0.85) - 3.5 * bell(3500, 0.95)
              - 4.0 * bell(9000, 1.5) - 1.0 * (t - 0.5) * 6.0)
        if f < 32:
            db -= 10.0 * math.log2(32.0 / max(20.0, f))
        pts.append((a[0] + t * w, mid - (db / 14.0) * (h * 0.46)))
    pline(d, pts, SOFT, 1.5)
    for (bx, by) in (pts[10], pts[28], pts[58], pts[84]):
        d.ellipse([px(bx - 2.6), px(by - 2.6), px(bx + 2.6), px(by + 2.6)],
                  fill=(255, 245, 242), outline=ACC, width=px(1))


def art_comp(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    left = (a[0], a[1], a[0] + w * 0.56, a[3])
    d.rounded_rectangle([px(left[0]), px(left[1]), px(left[2]), px(left[3])], radius=px(4),
                        fill=(9, 10, 13), outline=(27, 30, 36), width=px(1))
    for i in range(1, 4):
        dline(d, left[0], left[1] + (left[3] - left[1]) * i / 4.0, left[2],
              left[1] + (left[3] - left[1]) * i / 4.0, (23, 26, 32), 1.0)
    pts = [(left[0] + 4 + (left[2] - left[0] - 8) * (i / 39.0) ** 0.62,
            left[3] - 3 - (left[3] - left[1] - 6) * (i / 39.0)) for i in range(40)]
    pline(d, pts, ACC, 1.4)

    gx = a[0] + w * 0.68
    text(d, (gx, a[1] + 4), "GR", 6.8, DIM, track=1.4, anchor="lm")
    bar = (gx, a[1] + 13, a[2], a[3] - 4)
    d.rounded_rectangle([px(bar[0]), px(bar[1]), px(bar[2]), px(bar[3])], radius=px(3),
                        fill=(9, 10, 13), outline=(27, 30, 36), width=px(1))
    d.rounded_rectangle([px(bar[0] + 1.5), px(bar[1] + 1.5),
                         px(bar[0] + (bar[2] - bar[0]) * 0.30), px(bar[3] - 1.5)],
                        radius=px(2), fill=ACC)
    text(d, (bar[2] - 2, bar[3] + 1), "\u22126.8", 6.6, SOFT, track=0.5, anchor="rt")


def art_reverb(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    pts = [(a[0] + (i / 59.0) * w,
            a[3] - 2 - math.exp(-(i / 59.0) * 2.6) * (h - 4)) for i in range(60)]
    pline(d, pts, SOFT, 1.5)
    dline(d, a[0], a[3] - 1, a[2], a[3] - 1, (28, 31, 38))


def art_delay(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    for c in range(9):
        amp = max(0.06, 0.62 ** c)
        x = a[0] + w * c / 8.0
        for r in range(2):
            y = a[1] + h * (0.32 + 0.38 * r)
            rr = 1.8 + 1.7 * amp
            d.ellipse([px(x - rr), px(y - rr), px(x + rr), px(y + rr)],
                      fill=tuple(int(v * min(1.0, amp * 1.6)) for v in SOFT))


def art_mod(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    pts = [(a[0] + (i / 79.0) * w,
            a[1] + h * (0.5 - 0.40 * math.sin(i / 79.0 * 4 * math.pi))) for i in range(80)]
    pline(d, pts, SOFT, 1.4)


def art_image(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    cx, cy = (a[0] + a[2]) / 2.0, a[3]
    r = min(w * 0.30, h * 1.05)
    d.arc([px(cx - r), px(cy - r), px(cx + r), px(cy + r)], 180, 360,
          fill=(42, 46, 54), width=px(1.2))
    for ang in range(185, 356, 20):
        dline(d, cx + math.cos(math.radians(ang)) * r * 0.55,
              cy + math.sin(math.radians(ang)) * r * 0.55,
              cx + math.cos(math.radians(ang)) * r * 0.94,
              cy + math.sin(math.radians(ang)) * r * 0.94, (54, 50, 52), 1.1)
    for ang in (210, 235, 260, 285, 310, 335):
        dline(d, cx, cy, cx + math.cos(math.radians(ang)) * r * 0.94,
              cy + math.sin(math.radians(ang)) * r * 0.94, ACC, 1.2)


def art_trns(d, a):
    w, h = a[2] - a[0], a[3] - a[1]
    pts = [(a[0] + (i / 59.0) * w,
            a[3] - 2 - (math.exp(-(i / 59.0) * 5.0) * 0.82
                        + (1 - math.exp(-(i / 59.0) * 12)) * 0.26) * (h - 4))
           for i in range(60)]
    pline(d, pts, SOFT, 1.4)
    dline(d, a[0], a[3] - 1, a[2], a[3] - 1, (28, 31, 38))


def art_io(d, a):
    for i in range(1, 4):
        dline(d, a[0], a[1] + (a[3] - a[1]) * i / 4.0, a[2],
              a[1] + (a[3] - a[1]) * i / 4.0, (23, 26, 32), 1.0)
    dline(d, a[0], a[1] + (a[3] - a[1]) / 2.0, a[2], a[1] + (a[3] - a[1]) / 2.0,
          (36, 40, 47), 1.0)


# ------------------------------------------------------------------ cards
def card_header(img, box, title, on=True, readout="", pills=(), switch=True,
                extra_switches=0):
    d = ImageDraw.Draw(img)
    x0, y0 = box[0], box[1]

    n = 0
    if switch:
        power_lamp(img, x0 + 13, y0 + 7, on)
        n = 1
    for i in range(extra_switches):
        power_switch(ImageDraw.Draw(img), x0 + 13 + 52 * (i + 1), y0 + 7, True,
                     w=48, h=18)

    d = ImageDraw.Draw(img)
    tx = x0 + 13 + 52 * (n + extra_switches)

    text(d, (tx, y0 + 16.5), title.upper(), 10.4, INK if on else MUT, track=2.3, anchor="lm")

    if readout:
        text(d, (box[2] - 14, y0 + 16.5), readout, 7.8, DIM, track=1.3, anchor="rm")

    dline(d, x0 + 13, y0 + 31, box[2] - 13, y0 + 31, (25, 28, 34), 1.0)

    for (x, label, onp) in pills:
        w = 42
        d.rounded_rectangle([px(x), px(y0 + 7), px(x + w), px(y0 + 25)], radius=px(9),
                            fill=(30, 16, 13) if onp else (17, 19, 23),
                            outline=ACC if onp else (47, 51, 59), width=px(1))
        text(d, (x + w / 2, y0 + 16.2), label, 6.8, SOFT if onp else DIM, track=1.2)


def module_card(img, box, key, title, readout="", art=None, on=True, pills=(),
                switch=True, knob_floor=40, extra_switches=0):
    card(img, box)
    card_header(img, box, title, on=on, readout=readout, pills=pills, switch=switch,
                extra_switches=extra_switches)

    knobs = MODULES[key]
    x0, x1 = box[0] + 13, box[2] - 13
    slot = (x1 - x0) / float(len(knobs))
    dd = max(knob_floor, min(56.0, slot - 12.0))
    r = dd / 2.0 - 1.0
    block = dd + KNOB_GAP_LABEL + 4

    row_top = box[3] - 7 - block
    art_box = (x0, box[1] + CARD_HEAD + 4, x1, row_top - 4)
    if art is not None and (art_box[3] - art_box[1]) >= 16:
        art(ImageDraw.Draw(img), art_box)

    for i, (label, value, frac) in enumerate(knobs):
        cx = x0 + slot * (i + 0.5)
        cy = row_top + r + 1
        draw_knob(img, cx, cy, r, label, value, frac,
                  pointer="red" if key in ("IO", "COLOR") else "light",
                  label_size=min(10.0, slot * 0.20), value_size=min(9.4, slot * 0.19))

    return (x0, x1, row_top, art_box)


# ------------------------------------------------------------------ header
def monogram(img, x, y, h=28.0):
    d = ImageDraw.Draw(img)
    w = h * 0.40
    for k in range(2):
        ox = x + k * (w + h * 0.16)
        dline(d, ox, y + h, ox, y, INK, 1.7)
        dline(d, ox, y, ox + w, y + h * 0.60, INK, 1.7)
        dline(d, ox + w, y + h * 0.60, ox + w, y + h, INK, 1.7)
    dline(d, x + w + h * 0.08, y + h * 0.30, x + w + h * 0.08, y + h, ACC, 1.4)


def header(img, preset_name="001  Modern Clarity", category="SIGNATURE PRESET",
           active_tab="ALL"):
    grad = Image.new("RGB", img.size, (0, 0, 0))
    ImageDraw.Draw(grad).rectangle([0, 0, px(W), px(HEADER_BAND - 1)], fill=(10, 11, 14))
    img.paste(ImageChops.add(img, grad), (0, 0))

    d = ImageDraw.Draw(img)
    dline(d, 0, HEADER_BAND - 1, W, HEADER_BAND - 1, (26, 29, 35), 1.0)

    monogram(img, 24, 20, 26)
    dline(d, 84, 12, 84, 54, (33, 37, 44), 1.0)
    text(d, (96, 26), "MH SIGNATURE", 13.4, INK, track=4.0, anchor="lm")
    text(d, (97, 44), "BY MILAD HASHEMI", 7.2, DIM, track=3.2, anchor="lm")

    tabs = ["ALL", "EQ", "DYN", "COLOR", "SPACE", "MOD", "IMAGE"]
    widths = [44, 40, 42, 60, 62, 46, 66]
    total = sum(widths) + 20 * (len(tabs) - 1)
    x = 780 - total / 2.0
    for label, wd in zip(tabs, widths):
        cx = x + wd / 2.0
        on = label == active_tab
        text(d, (cx, 29), label, 8.8, INK if on else MUT, track=2.1)
        if on:
            glow(img, lambda dd_, cx=cx, wd=wd: dd_.line(
                [px(cx - wd / 2), px(44), px(cx + wd / 2), px(44)], fill=ACC,
                width=px(1.6)), blur=5.0, gain=0.9)
            d = ImageDraw.Draw(img)
            dline(d, cx - wd / 2, 44, cx + wd / 2, 44, ACC, 1.6)
        x += wd + 20

    pbox = (1124, 13, 1350, 55)
    d.rounded_rectangle([px(pbox[0]), px(pbox[1]), px(pbox[2]), px(pbox[3])], radius=px(6),
                        fill=(11, 12, 15), outline=(38, 42, 49), width=px(1))
    text(d, (pbox[0] + 12, pbox[1] + 12.5), category, 6.8, DIM, track=1.8, anchor="lm")
    text(d, (pbox[0] + 12, pbox[1] + 29), preset_name, 11.4, INK2, track=0.0, anchor="lm")
    dline(d, pbox[2] - 22, pbox[1] + 22, pbox[2] - 18, pbox[1] + 26, MUT, 1.3)
    dline(d, pbox[2] - 18, pbox[1] + 26, pbox[2] - 14, pbox[1] + 22, MUT, 1.3)

    text(d, (1426, 25), "SOUND WITH", 6.8, DIM, track=2.6, anchor="rm")
    text(d, (1426, 40), "INTENTION", 6.8, DIM, track=2.6, anchor="rm")


def footer(img):
    d = ImageDraw.Draw(img)
    dline(d, 0, 864, W, 864, (25, 28, 34), 1.0)
    text(d, (310, 878), "VERSION 1.0.0", 7.2, FAINT, track=2.6, anchor="lm")
    text(d, (836, 878), "AN ALL-IN-ONE MIXING ENVIRONMENT", 8.8, (172, 179, 188),
         track=3.0, anchor="mm")
    text(d, (1426, 878), "DESIGNED BY MILAD HASHEMI", 7.2, FAINT, track=2.6, anchor="rm")


# ------------------------------------------------------------------ portrait
def portrait_rail(img):
    box = R_PORTRAIT
    tw, th = px(box[2] - box[0]), px(box[3] - box[1])
    aspect = tw / float(th)

    src = Image.open(PORTRAIT).convert("RGB")
    sw, sh = src.size
    ch = int(sh * 0.955)
    cw = int(ch * aspect)
    cx = int(sw * 0.555)
    top = int(sh * 0.015)
    x0 = max(0, min(sw - cw, cx - cw // 2))
    face = src.crop((x0, top, x0 + cw, top + ch)).resize((tw, th), Image.LANCZOS)

    arr = np.asarray(face).astype(np.float32)
    yy = np.linspace(0.0, 1.0, th)[:, None, None]
    xx = np.linspace(0.0, 1.0, tw)[None, :, None]

    bottom = np.clip((yy - 0.70) / 0.30, 0.0, 1.0) ** 1.5
    top_fade = np.clip((0.08 - yy) / 0.08, 0.0, 1.0) ** 1.2
    right = np.clip((xx - 0.84) / 0.16, 0.0, 1.0) ** 1.5
    veil = np.clip(bottom * 0.95 + top_fade * 0.65 + right * 0.5, 0.0, 1.0)
    arr = arr * (1.0 - veil) + np.array(NOIR, dtype=np.float32) * veil

    # a breath of vermilion light along the left edge, from the lamp of the desk
    arr[:, :px(26), 0] = np.clip(arr[:, :px(26), 0] * 1.06 + 12, 0, 255)
    img.paste(Image.fromarray(arr.clip(0, 255).astype(np.uint8)), (px(box[0]), px(box[1])))

    x0d, x1d = box[0], box[2]
    d = ImageDraw.Draw(img)

    script(img, (x0d + x1d) / 2.0, 640, "Milad Hashemi", 21.0, (247, 241, 235))
    glow(img, lambda dd: dd.line(
        [px(x0d + 40), px(662), px(x0d + 96), px(658), px(x0d + 156), px(664)],
        fill=ACC, width=px(1.2)), blur=5.0, gain=0.7)
    d = ImageDraw.Draw(img)
    dline(d, x0d + 40, 662, x0d + 96, 658, ACC, 1.2)
    dline(d, x0d + 96, 658, x0d + 156, 664, ACC, 1.2)
    text(d, ((x0d + x1d) / 2.0, 686), "MUSIC  ·  PEOPLE  ·  EMOTION  ·  ALWAYS", 6.6,
         (198, 190, 184), track=2.4)
    text(d, ((x0d + x1d) / 2.0, 814), "VERSION 1.0.0", 6.6, FAINT, track=3.0)

    dline(d, x1d + 0.5, box[1], x1d + 0.5, box[3], (28, 31, 37), 1.0)


# ------------------------------------------------------------------ meters
def meter_card(img, box, title, level_db, mirrored):
    card(img, box)
    d = ImageDraw.Draw(img)
    cx = (box[0] + box[2]) / 2.0

    text(d, (cx, box[1] + 16), title, 7.8, INK2, track=2.6)

    top, bot = box[1] + 34, box[3] - 30
    bar_w = 8.0
    bar_x = cx - bar_w / 2.0

    d.rounded_rectangle([px(bar_x - 4), px(top - 2), px(bar_x + bar_w + 4), px(bot + 2)],
                        radius=px(5), fill=(8, 9, 12), outline=(28, 31, 38), width=px(1))

    lab_x = bar_x - 9 if mirrored else bar_x + bar_w + 8
    align = "rm" if mirrored else "lm"
    for db in range(0, -61, -6):
        y = bot - (db + 60.0) / 60.0 * (bot - top)
        dline(d, bar_x - 7, y, bar_x - 4, y, (58, 63, 72), 1.0)
        text(d, (lab_x, y), str(db).replace("-", "\u2212"), 6.2, DIM, track=0.3, anchor=align)

    n = (level_db + 60.0) / 60.0
    if n > 0.001:
        y0 = bot - n * (bot - top)
        glow(img, lambda dd: dd.rectangle([px(bar_x - 1), px(y0), px(bar_x + bar_w + 1),
                                           px(bot)], fill=(120, 30, 16)), blur=2.2, gain=1.0)
        d = ImageDraw.Draw(img)
        seg, y = 3.2, bot
        while y > y0:
            ytop = max(y0, y - seg)
            hot = (bot - y0) < (bot - top) * 0.10 and y < y0 + 14
            if y - 1.0 > ytop:
                d.rectangle([px(bar_x), px(ytop), px(bar_x + bar_w), px(y - 1.0)],
                            fill=SOFT if hot else ACC)
            y -= seg + 1.2
        dline(d, bar_x - 4, y0, bar_x + bar_w + 4, y0, SOFT, 1.2)

    text(d, (cx, box[3] - 15), f"{level_db:.1f} dB".replace("-", "\u2212"), 8.0, INK2, track=0.5)


# ------------------------------------------------------------------ spectrum
def spectrum_card(img, box):
    card(img, box)
    d = ImageDraw.Draw(img)

    tabs = ["SPECTRUM", "WAVEFORM", "LOUDNESS"]
    widths = [84, 88, 84]
    x = box[0] + 15
    for i, (label, wd) in enumerate(zip(tabs, widths)):
        text(d, (x, box[1] + 16.5), label, 8.2, INK if i == 0 else DIM, track=1.8, anchor="lm")
        if i == 0:
            dline(d, x - 2, box[1] + 27, x + wd - 16, box[1] + 27, ACC, 1.4)
        x += wd

    glow(img, lambda dd: dd.ellipse([px(box[2] - 74), px(box[1] + 13),
                                     px(box[2] - 68), px(box[1] + 19)], fill=ACC),
         blur=4.0, gain=0.9)
    d = ImageDraw.Draw(img)
    d.ellipse([px(box[2] - 74), px(box[1] + 12), px(box[2] - 67), px(box[1] + 19)], fill=ACC)
    text(d, (box[2] - 62, box[1] + 16.5), "PRE", 7.0, INK2, track=1.5, anchor="lm")
    d.ellipse([px(box[2] - 36), px(box[1] + 13), px(box[2] - 30), px(box[1] + 19)],
              outline=(72, 78, 88), width=px(1))
    text(d, (box[2] - 25, box[1] + 16.5), "POST", 7.0, DIM, track=1.5, anchor="lm")

    dline(d, box[0] + 13, box[1] + 32, box[2] - 13, box[1] + 32, (25, 28, 34))

    plot = (box[0] + 13, box[1] + 40, box[2] - 13, box[3] - 12)
    d.rounded_rectangle([px(plot[0]), px(plot[1]), px(plot[2]), px(plot[3])], radius=px(5),
                        fill=(8, 9, 12), outline=(26, 29, 35), width=px(1))

    inner = (plot[0] + 27, plot[1] + 7, plot[2] - 7, plot[3] - 17)
    for db in (-12, -24, -36, -48, -60):
        y = inner[3] - (db + 60) / 60.0 * (inner[3] - inner[1])
        dline(d, inner[0], y, inner[2], y, (22, 25, 31), 1.0)
        text(d, (plot[0] + 4, y), str(db).replace("-", "\u2212"), 6.2, (78, 84, 94),
             track=0.2, anchor="lm")

    for f, lab in ((20, "20"), (100, "100"), (500, "500"), (1000, "1k"), (5000, "5k"),
                   (10000, "10k"), (20000, "20k")):
        t = math.log(max(20.0, f) / 20.0) / math.log(1000.0)
        x = inner[0] + t * (inner[2] - inner[0])
        dline(d, x, inner[1], x, inner[3], (22, 25, 31), 1.0)
        text(d, (x, plot[3] - 8), lab, 6.2, (78, 84, 94), track=0.2)

    anchors = [(20, -44), (35, -30), (60, -18), (110, -14), (200, -18), (400, -24),
               (700, -27), (1200, -31), (2500, -35), (5000, -40), (9000, -46),
               (14000, -52), (20000, -60)]

    def anchor_db(f):
        lf = math.log(f)
        for (f0, d0), (f1, d1) in zip(anchors, anchors[1:]):
            if f0 <= f <= f1:
                t = (lf - math.log(f0)) / (math.log(f1) - math.log(f0))
                return d0 + (d1 - d0) * t
        return anchors[-1][1]

    pts = []
    for i in range(200):
        t = i / 199.0
        f = 20.0 * 1000.0 ** t
        v = anchor_db(f) + 1.6 * math.sin(t * 92.0) + 1.1 * math.sin(t * 27.0 + 1.2)
        pts.append((inner[0] + t * (inner[2] - inner[0]),
                    inner[3] - (max(-58.0, min(-3.0, v)) + 60.0) / 60.0 * (inner[3] - inner[1])))

    filled = [(inner[0], inner[3])] + pts + [(inner[2], inner[3])]
    mask = Image.new("L", img.size, 0)
    ImageDraw.Draw(mask).polygon(P(filled), fill=255)

    w, h = px(inner[2] - inner[0]), px(inner[3] - inner[1])
    ramp = np.repeat(np.linspace(62.0, 2.0, h)[:, None], w, axis=1).astype(np.float32)
    alpha = np.asarray(mask, dtype=np.float32)
    alpha[px(inner[1]):px(inner[3]), px(inner[0]):px(inner[2])] *= ramp / 255.0
    img.paste(Image.new("RGB", img.size, ACC), (0, 0),
              Image.fromarray(alpha.clip(0, 255).astype(np.uint8), "L"))

    d = ImageDraw.Draw(img)
    pline(d, pts, SOFT, 1.5)


# ------------------------------------------------------------------ engine
def engine_card(img, box):
    card(img, box, glow_amount=0.5)
    d = ImageDraw.Draw(img)

    title_y = box[1] + 20
    text(d, ((box[0] + box[2]) / 2.0, title_y), "SIGNATURE ENGINE", 10.0, INK, track=5.0)
    dline(d, box[0] + 13, box[1] + 33, box[2] - 13, box[1] + 33, (26, 29, 35))

    cx = (box[0] + box[2]) / 2.0
    cy_top = box[1] + 92
    cy_bot = box[1] + 192
    cy = (cy_top + cy_bot) / 2.0

    big_r = min (74.0, (box[3] - box[1] - 33 - 60) / 2.0, (box[2] - box[0]) * 0.23)
    draw_knob(img, cx, cy, big_r, "", "", 1.0, pointer="red", big=True)
    d = ImageDraw.Draw(img)
    text(d, (cx, cy + big_r + 14), "INTENSITY", 9.0, INK2, track=3.0)
    text(d, (cx, cy + big_r + 26), "100", 8.6, SOFT, track=1.0)

    macros = (("CHARACTER", "62", 0.62, box[0] + 82, cy_top),
              ("SPACE",     "70", 0.70, box[2] - 82, cy_top),
              ("DENSITY",   "48", 0.48, box[0] + 82, cy_bot),
              ("WIDTH",     "56", 0.56, box[2] - 82, cy_bot))

    for label, value, frac, mx, my in macros:
        draw_knob(img, mx, my, 25.0, label, value, frac, pointer="red",
                  label_size=8.4, value_size=8.0)

    text(d, ((box[0] + box[2]) / 2.0, box[3] - 13), "TURN IDEAS INTO SOUND",
         7.8, DIM, track=4.2)


# ------------------------------------------------------------------ build
def build():
    img = new_board()

    header(img)
    portrait_rail(img)

    meter_card(img, R_IN, "INPUT", -3.2, mirrored=True)
    meter_card(img, R_OUT, "OUTPUT", -1.0, mirrored=False)

    engine_card(img, R_ENGINE)
    spectrum_card(img, R_SPEC)

    module_card(img, R_EQ, "EQ", "EQ", readout="4 BANDS", art=art_eq)
    module_card(img, R_DYN, "DYN", "DYNAMICS", readout="GR \u22126.8", art=art_comp,
                extra_switches=1)
    module_card(img, R_COLOR, "COLOR", "SATURATION", readout="", art=None)

    # character chip: parameter name on the left, its value on the right
    d = ImageDraw.Draw(img)
    chip = (R_COLOR[2] - 137, R_COLOR[1] + 7, R_COLOR[2] - 13, R_COLOR[1] + 25)
    d.rounded_rectangle([px(chip[0]), px(chip[1]), px(chip[2]), px(chip[3])], radius=px(4),
                        fill=(30, 16, 13), outline=ACC, width=px(1))
    text(d, (chip[0] + 7, (chip[1] + chip[3]) / 2), "CHARACTER", 6.0, DIM, track=1.4,
         anchor="lm")
    text(d, (chip[2] - 7, (chip[1] + chip[3]) / 2), "CONSOLE", 6.6, SOFT, track=1.1,
         anchor="rm")

    d = ImageDraw.Draw(img)
    band = (R_COLOR[0] + 13, R_COLOR[1] + 40, R_COLOR[2] - 13, R_COLOR[1] + 66)
    pts = [(band[0] + (i / 89.0) * (band[2] - band[0]),
            (band[1] + band[3]) / 2.0
            - (band[3] - band[1]) * 0.40 * math.tanh(
                math.sin(i / 89.0 * 7 * math.pi) * 2.8)) for i in range(90)]
    pline(d, pts, SOFT, 1.4)
    dline(d, band[0], band[3] + 1, band[2], band[3] + 1, (26, 29, 35), 1.0)

    module_card(img, R_VOCAL, "VOCAL", "VOCAL CONTROL", readout="DE-ESS 6.5 k")
    module_card(img, R_REVERB, "REVERB", "REVERB", readout="3.4 s", art=art_reverb)
    module_card(img, R_DELAY, "DELAY", "DELAY", readout="375 ms", art=art_delay)
    module_card(img, R_IMAGE, "IMAGE", "IMAGER", readout="M/S 92", art=art_image)

    module_card(img, R_MOD, "MOD", "MODULATION", readout="0.30 Hz", art=art_mod)
    module_card(img, R_TRNS, "TRNS", "TRANSIENT", readout="180 Hz", art=art_trns)
    module_card(img, R_IO, "IO", "UTILITY / I/O", readout="", art=art_io,
                pills=((R_IO[2] - 100, "MONO", False), (R_IO[2] - 52, "BYPASS", False)),
                switch=False)

    footer(img)
    return img


if __name__ == "__main__":
    board = build()
    big = board if board.size == (W * S, H * S) else board.resize((W * S, H * S), Image.LANCZOS)
    big.save(os.path.join(HERE, "gui-31-signature.png"))
    board.resize((1440, 900), Image.LANCZOS).save(
        os.path.join(HERE, "chat-31-signature.jpg"), quality=90, optimize=True)
    print("wrote gui-31-signature.png", big.size)
