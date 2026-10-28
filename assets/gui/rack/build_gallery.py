#!/usr/bin/env python3
"""Builds gallery-rack.html — the ten new directions, as one shareable file."""
import base64
import os

from render import DESIGNS

HERE = os.path.dirname(os.path.abspath(__file__))


def data_uri(path):
    with open(path, "rb") as fh:
        return "data:image/png;base64," + base64.b64encode(fh.read()).decode()


CARDS = []
for num, name, fn, note in DESIGNS:
    slug = name.lower().replace(" ", "-")
    png = os.path.join(HERE, f"rack-{num}-{slug}.png")
    CARDS.append((num, name, note, data_uri(png)))

HTML = """<!DOCTYPE html>
<html lang="fa" dir="rtl">
<head>
<meta charset="utf-8">
<title>MH SIGNATURE — ۱۰ طرح رابط کاربری</title>
<style>
  :root {{
    --noir:#050607; --panel:#0e1014; --edge:#22262d; --acc:#d0361c;
    --soft:#f2745a; --ink:#eef1f5; --mut:#8a929e;
  }}
  * {{ box-sizing:border-box; }}
  body {{
    margin:0; background:var(--noir); color:var(--ink);
    font-family:"Vazirmatn","IRANSans","Segoe UI",Tahoma,sans-serif;
    padding:56px 40px 96px;
  }}
  header {{ max-width:1500px; margin:0 auto 44px; }}
  .kicker {{ color:var(--acc); letter-spacing:.34em; font-size:12px; margin:0 0 14px; }}
  h1 {{ font-size:clamp(26px,3.2vw,44px); margin:0 0 14px; font-weight:800; letter-spacing:-.5px; }}
  .lede {{ color:#b9c0c9; font-size:16px; line-height:1.95; max-width:1080px; margin:0; }}
  .lede b {{ color:var(--ink); }}
  .rules {{
    display:flex; gap:10px; flex-wrap:wrap; margin:26px 0 0; padding:0; list-style:none;
  }}
  .rules li {{
    border:1px solid var(--edge); background:#0b0d11; color:#c4cbd4;
    border-radius:999px; padding:8px 16px; font-size:13px;
  }}
  .rules li b {{ color:var(--soft); font-weight:700; }}
  .grid {{ max-width:1500px; margin:0 auto; display:grid; gap:56px; }}
  .card {{ border-top:1px solid var(--edge); padding-top:22px; }}
  .head {{ display:flex; align-items:baseline; gap:16px; flex-wrap:wrap; margin-bottom:6px; }}
  .num {{
    color:var(--acc); font-size:13px; font-weight:800; letter-spacing:.2em;
    border:1px solid #3a2018; background:#170c09; border-radius:6px; padding:3px 9px;
  }}
  .name {{ font-size:22px; font-weight:800; margin:0; letter-spacing:.02em; }}
  .note {{ color:#a8b0ba; font-size:14.5px; line-height:1.85; margin:6px 0 16px; }}
  .shot {{ border:1px solid var(--edge); border-radius:10px; overflow:hidden; background:#000; }}
  .shot img {{ width:100%; display:block; }}
  footer {{ max-width:1500px; margin:64px auto 0; color:var(--mut); font-size:13px; line-height:2; }}
</style>
</head>
<body>
<header>
  <p class="kicker">MH SIGNATURE · MIRROR · RACK</p>
  <h1>۱۰ طرح تازه برای رابط کاربری</h1>
  <p class="lede">
    همهٔ طرح‌ها با یک قانون مشترک کشیده شده‌اند: <b>نوشته‌ها درشت و خوانا در اندازهٔ ۱۰۰٪</b>،
    هر ماژول یک <b>کلید ON/OFF</b> مستقل دارد، و چیدمان به‌صورت یک واحد مقیاس می‌شود
    تا هنگام بزرگ‌کردن پنجره هیچ‌چیز به‌هم نریزد. پالت همان است: مشکی،
    <b>#d0361c</b> و بدون هیچ طلایی.
  </p>
  <ul class="rules">
    <li>برچسب نُب‌ها <b>٫</b> درشت‌تر</li>
    <li>کلید <b>ON / OFF</b> برای هر ماژول</li>
    <li>مقیاس <b>۸۵٪ تا ۲۰۰٪</b></li>
    <li>پرتره <b>بخشی از چیدمان</b></li>
    <li>بدون <b>طلایی</b></li>
  </ul>
</header>
<div class="grid">
{cards}
</div>
<footer>
  شمارهٔ طرح را بگو (مثلاً «۴» یا «۱۰») تا همان را در کد پیاده کنم.
  رنگ، فاصله‌ها و اندازهٔ نوشته‌ها در همهٔ طرح‌ها یکسان است؛ تفاوت فقط در چیدمان است.
</footer>
</body>
</html>
"""


def card_html(num, name, note, uri):
    return f"""  <section class="card">
    <div class="head"><span class="num">{num}</span><h2 class="name">{name}</h2></div>
    <p class="note">{note}</p>
    <div class="shot"><img src="{uri}" alt="{name}"></div>
  </section>"""


def main():
    html = HTML.format(cards="\n".join(card_html(*c) for c in CARDS))
    out = os.path.join(HERE, "gallery-rack.html")
    with open(out, "w", encoding="utf-8") as fh:
        fh.write(html)
    print(f"wrote {out} — {os.path.getsize(out)/1048576:.1f} MB")


if __name__ == "__main__":
    main()
