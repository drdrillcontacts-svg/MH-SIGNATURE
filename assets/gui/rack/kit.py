#!/usr/bin/env python3
"""Renders 10 minimal MH SIGNATURE GUI directions at 2x for review.

Every board is drawn with the same primitives the JUCE build will use:
1 px hairlines, #D0361C accent, 11 px labels, a real ON/OFF pill for each
module and the portrait as a working part of the layout.
"""
import math
import os
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter

S = 2                      # supersampling factor
W, H = 1440, 900           # design space, identical to the plugin

NOIR   = (5, 6, 7)
PANEL  = (16, 18, 22)
PANEL2 = (22, 25, 30)
EDGE   = (34, 37, 43)
EDGE2  = (48, 52, 60)
ACC    = (208, 54, 28)
DEEP   = (143, 36, 18)
SOFT   = (242, 116, 90)
INK    = (238, 241, 245)
INK2   = (200, 206, 214)
MUT    = (132, 140, 152)
DIM    = (86, 93, 104)

FDIR = "/usr/share/fonts/truetype/dejavu/"
F_SANS   = FDIR + "DejaVuSans.ttf"
F_BOLD   = FDIR + "DejaVuSans-Bold.ttf"
F_SERIF  = FDIR + "DejaVuSerif.ttf"
F_MONO   = FDIR + "DejaVuSansMono.ttf"

_cache = {}


def font(path, size):
    key = (path, size)
    if key not in _cache:
        _cache[key] = ImageFont.truetype(path, max(6, int(round(size * S))))
    return _cache[key]


def px(v):
    return int(round(v * S))


# --------------------------------------------------------------------------- primitives
def new_board():
    return Image.new("RGB", (W * S, H * S), NOIR)


def rr(d, box, radius=6, fill=None, outline=None, width=1):
    x0, y0, x1, y1 = [px(v) for v in box]
    d.rounded_rectangle([x0, y0, x1, y1], radius=px(radius), fill=fill,
                        outline=outline, width=max(1, px(width)) if outline else 0)


def line(d, x0, y0, x1, y1, colour, width=1):
    d.line([px(x0), px(y0), px(x1), px(y1)], fill=colour, width=max(1, px(width)))


def circle(d, cx, cy, r, fill=None, outline=None, width=1):
    d.ellipse([px(cx - r), px(cy - r), px(cx + r), px(cy + r)], fill=fill,
              outline=outline, width=max(1, px(width)) if outline else 0)


def text(d, xy, s, size, colour=INK, bold=False, anchor="lm", track=0.0,
         serif=False, mono=False):
    path = F_MONO if mono else (F_SERIF if serif else (F_BOLD if bold else F_SANS))
    f = font(path, size)

    if track <= 0.0:
        d.text((px(xy[0]), px(xy[1])), s, font=f, fill=colour, anchor=anchor)
        return

    # manual letter spacing
    widths = [d.textlength(ch, font=f) for ch in s]
    total = sum(widths) + track * S * (len(s) - 1)
    x = px(xy[0])
    if anchor[0] == "m":
        x -= total / 2.0
    elif anchor[0] == "r":
        x -= total

    for ch, w in zip(s, widths):
        d.text((x, px(xy[1])), ch, font=f, fill=colour, anchor="l" + anchor[1])
        x += w + track * S


def arc(d, cx, cy, r, a0, a1, colour, width=2):
    d.arc([px(cx - r), px(cy - r), px(cx + r), px(cy + r)],
          start=a0, end=a1, fill=colour, width=max(1, px(width)))


def pill(d, box, on, label=None, size=8.5):
    """The module ON/OFF switch — same in every direction."""
    x, y, x1, y1 = box
    w, h = x1 - x, y1 - y
    rr(d, box, radius=h / 2, fill=(26, 21, 19) if on else (18, 21, 26),
       outline=(ACC if on else EDGE2), width=1)
    dot = h * 0.28
    if on:
        circle(d, x + h * 0.55, y + h / 2, dot * 1.9, fill=(46, 26, 20))
        circle(d, x + h * 0.55, y + h / 2, dot, fill=ACC)
    else:
        circle(d, x + h * 0.55, y + h / 2, dot, outline=DIM, width=1)
    text(d, (x + h * 1.15, y + h / 2 + 0.3), label or ("ON" if on else "OFF"), size,
         INK if on else DIM, bold=True, anchor="lm", track=0.6)


def knob(d, cx, cy, r, label, value, frac, big=False, label_size=None, value_size=None):
    a0 = 135.0
    a1 = 405.0
    sweep = (a1 - a0) * frac

    arc(d, cx, cy, r - 1.6, a0, a1, (43, 47, 54), 2.6 if big else 2.2)
    if frac > 0.004:
        arc(d, cx, cy, r - 1.6, a0, a0 + sweep, ACC, 2.8 if big else 2.4)

    body = r - (10 if big else 6.5)
    circle(d, cx, cy, body + 0.8, fill=(33, 36, 43))
    circle(d, cx, cy, body, fill=(11, 12, 15))
    circle(d, cx - body * 0.25, cy - body * 0.3, body * 0.55, fill=(28, 31, 37))

    ang = math.radians(a0 + sweep)
    r0, r1 = body * 0.3, body * 0.86
    line(d, cx + math.cos(ang) * r0, cy + math.sin(ang) * r0,
         cx + math.cos(ang) * r1, cy + math.sin(ang) * r1, (232, 227, 222), 2.0 if big else 1.6)

    ls = label_size or (10.0 if big else 9.6)
    vs = value_size or (10.0 if big else 9.0)
    text(d, (cx, cy + r + 9), label, ls, (214, 220, 227), anchor="mm", track=0.9)
    text(d, (cx, cy + r + 9 + ls + 4), value, vs, SOFT, anchor="mm", mono=False)


def spectrum(d, box, detail=True, seed=3):
    x0, y0, x1, y1 = box
    rr(d, box, radius=4, fill=(8, 10, 13), outline=(26, 30, 36), width=1)
    rng = np.random.default_rng(seed)
    n = 110
    xs = np.linspace(x0 + 6, x1 - 6, n)

    def level(t):
        base = -20 + 6 * math.sin(t * 2.1) - 16 * t
        bump = 9 * math.exp(-((t - 0.22) ** 2) / 0.006) - 7 * math.exp(-((t - 0.55) ** 2) / 0.01)
        jag = rng.normal(0, 1.7)
        return max(-58, min(-4, base + bump + jag))

    vals = [level(i / (n - 1)) for i in range(n)]
    top = y0 + 12
    bot = y1 - 18

    def ypos(v):
        return bot - (v + 60) / 60.0 * (bot - top)

    pts = [(xs[i], ypos(vals[i])) for i in range(n)]
    if detail:
        for f, lab in ((0.0, "20"), (0.35, "200"), (0.65, "2k"), (1.0, "20k")):
            gx = x0 + 6 + f * (x1 - x0 - 12)
            line(d, gx, top, gx, bot, (26, 30, 36), 1)
            text(d, (gx, y1 - 8), lab, 7.2, DIM, anchor="mm")

    poly = [(px(x), px(y)) for x, y in pts] + [(px(xs[-1]), px(bot)), (px(xs[0]), px(bot))]
    glow = Image.new("RGB", (W * S, H * S), (0, 0, 0))
    gd = ImageDraw.Draw(glow)
    gd.polygon(poly, fill=(38, 12, 8))
    d.polygon(poly, fill=(30, 10, 7))
    d.line([(px(x), px(y)) for x, y in pts], fill=SOFT, width=max(1, px(1.2)))


def meter(d, box, level_db, side="in", ticks=True):
    x0, y0, x1, y1 = box
    w = x1 - x0
    barw = 9.0
    bx = x0 + (8.0 if side == "in" else w - 8.0 - barw)
    by0, by1 = y0 + 20, y1 - 4

    rr(d, (bx - 2, by0, bx + barw + 2, by1), radius=2.5, fill=(12, 14, 18),
       outline=(28, 32, 39), width=1)

    def ypos(db):
        return by1 - (db + 60) / 60.0 * (by1 - by0)

    if ticks:
        for db in range(0, -61, -6):
            y = ypos(db)
            line(d, bx - 3 if side == "in" else bx + barw + 1, y,
                 bx - 1 if side == "in" else bx + barw + 3, y, (44, 48, 55), 1)
            text(d, (bx + barw + 6 if side == "in" else bx - 6, y), f"{db}", 7.4, DIM,
                 anchor="lm" if side == "in" else "rm")

    y = ypos(level_db)
    for i in range(int(by1 - y)):
        f = i / max(1.0, by1 - y)
        col = (int(ACC[0] + (SOFT[0] - ACC[0]) * f),
               int(ACC[1] + (SOFT[1] - ACC[1]) * f),
               int(ACC[2] + (SOFT[2] - ACC[2]) * f))
        line(d, bx, by1 - i, bx + barw, by1 - i, col, 1)
    text(d, (x0 if side == "in" else x1, y0 + 8),
         f"{level_db:.1f} dB", 9.0, INK2, bold=True,
         anchor="lm" if side == "in" else "rm")


def portrait(box, radius=0, fade_bottom=0.55, fade_right=0.0, tint=0.55, dim=0.92, fade_left=0.0):
    x0, y0, x1, y1 = box
    w, h = px(x1 - x0), px(y1 - y0)
    src = Image.open(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                 "..", "..", "..", "plugin", "assets", "portrait.jpg")).convert("RGB")
    sw, sh = src.size
    scale = max(w / sw, h / sh)
    src = src.resize((int(sw * scale) + 2, int(sh * scale) + 2), Image.LANCZOS)
    left = (src.width - w) // 2
    top = max(0, int((src.height - h) * 0.30))
    crop = src.crop((left, top, left + w, top + h)).convert("RGB")
    arr = np.asarray(crop).astype(np.float32)

    # push it towards the vermilion signature without going pink
    arr[..., 0] = np.clip(arr[..., 0] * (1.0 + tint * 0.34), 0, 255)
    arr[..., 1] = np.clip(arr[..., 1] * (1.0 - tint * 0.16), 0, 255)
    arr[..., 2] = np.clip(arr[..., 2] * (1.0 - tint * 0.22), 0, 255)
    arr *= dim

    yy = np.linspace(0, 1, h)[:, None, None]
    xx = np.linspace(0, 1, w)[None, :, None]

    mask = np.ones((h, w, 1), np.float32)
    if fade_bottom > 0:
        mask *= np.clip((1.0 - yy) / max(0.001, 1.0 - (1.0 - fade_bottom)) * 1.0, 0, 1) ** 1.4
    if fade_right > 0:
        mask *= np.clip(xx / max(0.001, fade_right), 0, 1) ** 1.2
    if fade_left > 0:
        mask *= np.clip((1.0 - xx) / max(0.001, fade_left), 0, 1) ** 1.4

    img = Image.fromarray(np.clip(arr, 0, 255).astype(np.uint8))

    if radius > 0:
        m = Image.new("L", (w, h), 0)
        ImageDraw.Draw(m).rounded_rectangle([0, 0, w - 1, h - 1], radius=px(radius), fill=255)
        a = Image.fromarray((mask[..., 0] * 255 * (np.asarray(m) / 255.0)).astype(np.uint8))
    else:
        a = Image.fromarray((mask[..., 0] * 255).astype(np.uint8))

    return img, a, (px(x0), px(y0))


def paste_portrait(board, *args, **kwargs):
    img, mask, pos = portrait(*args, **kwargs)
    board.paste(img, pos, mask)


def header(board, d, style="full", title="MH SIGNATURE", sub="BY MILAD HASHEMI",
           tabs_at=None, preset=None, tagline="SOUND WITH INTENTION", portrait_tile=None):
    # wordmark
    if portrait_tile:
        paste_portrait(board, portrait_tile[0], radius=3, fade_bottom=0.0, tint=0.5)

    x = 24 if not portrait_tile else portrait_tile[0][2] + 16
    if not portrait_tile:
        text(d, (24, 22), "MH", 24, INK, bold=True, anchor="lm")
        line(d, 76, 12, 76, 44, (45, 49, 56), 1)
        x = 94

    text(d, (x, 22), title, 16, INK, anchor="lm", track=3.2)
    text(d, (x + 1, 42), sub, 8.6, MUT, anchor="lm", track=2.6)

    if tabs_at:
        labels = ["ALL", "EQ", "DYN", "COLOR", "SPACE", "MOD", "IMAGE"]
        tx, ty, tw_all, th = tabs_at
        tw = tw_all / len(labels)
        rr(d, (tx, ty, tx + tw_all, ty + th), radius=th / 2, fill=(11, 13, 17),
           outline=(30, 34, 41), width=1)
        for i, lab in enumerate(labels):
            bx = tx + i * tw
            if i == 0:
                rr(d, (bx + 3, ty + 3, bx + tw - 3, ty + th - 3),
                   radius=(th - 6) / 2, fill=(178, 45, 23))
            text(d, (bx + tw / 2, ty + th / 2), lab, 8.2,
                 (250, 226, 219) if i == 0 else MUT, bold=True, anchor="mm", track=1.6)

    if preset:
        x0, y0, x1, y1 = preset
        rr(d, preset, radius=(y1 - y0) / 2, fill=(17, 20, 25), outline=(34, 38, 45), width=1)
        text(d, (x0 + 16, y0 + 12), "SIGNATURE PRESET · 001", 7.4, (140, 148, 160),
             anchor="lm", track=1.6)
        text(d, ((x0 + x1) / 2, y0 + 27), "Modern Clarity", 12.6, INK, anchor="mm")

    if tagline:
        # never collide with the preset pill
        px1 = preset[2] if preset else 0
        if px1 < 1180:
            text(d, (W - 24, 26), tagline, 8.4, (150, 157, 168), anchor="rm", track=2.6)


def footer(d, left="VERSION 1.0.0", centre="AN ALL-IN-ONE MIXING ENVIRONMENT",
           right="DESIGNED BY MILAD HASHEMI"):
    line(d, 0, 866, W, 866, (26, 29, 35), 1)
    if left:
        text(d, (24, 882), left, 8.2, DIM, anchor="lm", track=2.2)
    if centre:
        text(d, (W / 2, 882), centre, 9.6, (206, 212, 220), anchor="mm", track=2.8)
    if right:
        text(d, (W - 24, 882), right, 8.2, DIM, anchor="rm", track=2.2)


def signature(d, x, y, size=40, colour=(244, 237, 231), flourish=True):
    text(d, (x, y), "Milad", size, colour, serif=True, anchor="mm")
    if flourish:
        w = size * 1.9
        pts = []
        for i in range(42):
            t = i / 41.0
            fx = x - w / 2 + t * w
            fy = y + size * 0.52 + math.sin(t * 3.4) * 3.0 - t * 1.2
            pts.append((px(fx), px(fy)))
        d.line(pts, fill=(178, 45, 23), width=max(1, px(1.2)))
