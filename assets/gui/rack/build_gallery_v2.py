#!/usr/bin/env python3
"""Builds gallery-v2.html — the five new directions, one shareable file."""
import base64
import os

from render_v2 import DESIGNS

HERE = os.path.dirname(os.path.abspath(__file__))
NOTES_EN = {
    "21": "Measuring instrument. Hairline ticks, monospaced numerals, one huge analyser. No cards at all.",
    "22": "Editorial spread. Full-height portrait on the left page, serif masthead, modules as a contents list.",
    "23": "Hardware rack. Flat grey plates, engraved labels, ten identical modules, zero gradients.",
    "24": "Poster. A 128 px numeral, halftone portrait, modules as tickets with value bars.",
    "25": "Modern bento. Unequal tiles, lots of air, one large analyser.",
}


def data_uri(path):
    with open(path, "rb") as fh:
        return "data:image/png;base64," + base64.b64encode(fh.read()).decode()


cards = []
for num, name, fn, note in DESIGNS:
    slug = name.lower().replace(" ", "-")
    uri = data_uri(os.path.join(HERE, f"gui-{num}-{slug}.png"))
    cards.append((num, name, note, NOTES_EN[num], uri))

HTML = """<!DOCTYPE html>
<html lang="fa" dir="rtl">
<head>
<meta charset="utf-8">
<title>MH SIGNATURE — پنج طرح تازه</title>
<style>
  :root {{ --noir:#050607; --edge:#22262d; --acc:#d0361c; --soft:#f2745a;
           --ink:#eef1f5; --mut:#8a929e; }}
  * {{ box-sizing:border-box; }}
  body {{ margin:0; background:var(--noir); color:var(--ink);
          font-family:"Vazirmatn","IRANSans","Segoe UI",Tahoma,sans-serif;
          padding:56px 40px 96px; }}
  header {{ max-width:1500px; margin:0 auto 46px; }}
  .kicker {{ color:var(--acc); letter-spacing:.34em; font-size:12px; margin:0 0 14px; }}
  h1 {{ font-size:clamp(26px,3.2vw,44px); margin:0 0 16px; font-weight:800; }}
  .lede {{ color:#b9c0c9; font-size:16px; line-height:2; max-width:1060px; margin:0; }}
  .lede b {{ color:var(--ink); }}
  .portrait {{ margin:28px 0 0; display:flex; gap:18px; align-items:center; }}
  .portrait img {{ width:96px; height:120px; object-fit:cover; border:1px solid var(--edge);
                   border-radius:8px; }}
  .portrait p {{ margin:0; color:#98a0aa; font-size:13.5px; line-height:1.9; }}
  .grid {{ max-width:1500px; margin:0 auto; display:grid; gap:64px; }}
  .card {{ border-top:1px solid var(--edge); padding-top:24px; }}
  .head {{ display:flex; align-items:baseline; gap:16px; flex-wrap:wrap; }}
  .num {{ color:var(--acc); font-size:13px; font-weight:800; letter-spacing:.2em;
          border:1px solid #3a2018; background:#170c09; border-radius:6px; padding:3px 9px; }}
  .name {{ font-size:23px; font-weight:800; margin:0; letter-spacing:.02em; }}
  .lang {{ color:var(--mut); font-size:12.5px; letter-spacing:.14em; text-transform:uppercase; }}
  .note {{ color:#a8b0ba; font-size:14.5px; line-height:1.95; margin:10px 0 18px; max-width:1100px; }}
  .shot {{ border:1px solid var(--edge); border-radius:10px; overflow:hidden; background:#000; }}
  .shot img {{ width:100%; display:block; }}
  footer {{ max-width:1500px; margin:64px auto 0; color:var(--mut); font-size:13px; line-height:2; }}
</style>
</head>
<body>
<header>
  <p class="kicker">MH SIGNATURE · ROUND TWO</p>
  <h1>پنج طرح، پنج زبان بصری</h1>
  <p class="lede">
    این بار هر طرح از یک <b>زبان طراحی متفاوت</b> شروع شده، نه از یک چیدمان تازه:
    ابزار اندازه‌گیری، مجله، سخت‌افزار، پوستر و بنتو. در هر پنج طرح:
    برچسب هر نُب خوانا، <b>کلید ON/OFF</b> مستقل برای هر ماژول، پرترهٔ جدید
    به‌عنوان بخشی از رابط، و پالت ثابت <b>#d0361c</b> بدون هیچ طلایی.
  </p>
  <div class="portrait">
    <img src="PORTRAIT_URI" alt="portrait">
    <p>
      پرترهٔ جدید رنگ‌بندی شد و در همهٔ طرح‌ها به‌کار رفت. نسخهٔ رنگی و نسخهٔ
      تک‌رنگ (برای طرح‌های سخت‌افزاری و پوستری) هر دو آماده است.
    </p>
  </div>
</header>
<div class="grid">
{cards}
</div>
<footer>
  یک شماره را انتخاب کن، یا ترکیبی بگو — مثلاً «زبان ۲۳ با چیدمان ۲۵».
  بعد از انتخاب، همان را در کد پیاده می‌کنم و بیلد تازه را می‌فرستم.
</footer>
</body>
</html>
"""


def card_html(num, name, note, en, uri):
    return f"""  <section class="card">
    <div class="head"><span class="num">{num}</span><h2 class="name">{name}</h2>
      <span class="lang">{en}</span></div>
    <p class="note">{note}</p>
    <div class="shot"><img src="{uri}" alt="{name}"></div>
  </section>"""


def main():
    portrait_uri = data_uri(os.path.join(HERE, "..", "..", "brand", "portrait-2026-vermilion.png"))
    html = HTML.replace("PORTRAIT_URI", portrait_uri)
    html = html.replace("{cards}", "\n".join(card_html(*c) for c in cards))
    out = os.path.join(HERE, "gallery-v2.html")
    with open(out, "w", encoding="utf-8") as fh:
        fh.write(html)
    print(f"wrote {out} — {os.path.getsize(out)/1048576:.1f} MB")


if __name__ == "__main__":
    main()
