#!/usr/bin/env python3
"""MH SIGNATURE — v3 gallery: 10 further GUI directions."""
import base64, os

OPTS = [
    ("v3-01-split-mirror", "SPLIT MIRROR", "آینهٔ دو نیمه",
     "یک درز عمودی، پنجره را دو نیم می‌کند. چپ: پرترهٔ تمام‌قد، نور قرمز از راست، محو در سیاه. "
     "راست: شبکهٔ فشردهٔ ماژول‌ها با نوارهای افقی.", "grid", 2),
    ("v3-02-fullbleed-ghost", "FULL-BLEED GHOST", "پرترهٔ سراسری محو",
     "چهرهٔ او تمام پنجره را پر می‌کند اما تقریباً در سیاه حل شده؛ فقط لبهٔ قرمزِ گونه و عینک بیرون می‌زند. "
     "کارت‌های شیشه‌ای شفاف روی آن شناورند.", "stagger", 2),
    ("v3-03-contact-sheet", "CONTACT SHEET", "برگهٔ نمونهٔ مجله‌ای",
     "پرتره در یک قاب عکس کج با حاشیهٔ قرمز و کپشن دست‌نویس؛ بقیهٔ رابط یک برگهٔ مشخصات با "
     "هفت ردیف اسلایدر افقی و عددهایی به سبک مونواسپیس.", "rows", 1),
    ("v3-04-orbit-ring", "ORBIT RING", "حلقهٔ مداری",
     "پرتره در یک روزنهٔ دایره‌ای با رینگ LED قطعه‌قطعه؛ هفت پاد ماژول با پره‌های نازک قرمز "
     "دور آن چیده شده‌اند.", "orbit", 1),
    ("v3-05-rack-vertical", "RACK VERTICAL", "رَک عمودی ۵۰۰-سری",
     "پرتره در یک اسلات شیشهٔ دودی در ستون چپ. هفت نوار عمودی کنار هم، هر کدام یک ناب سخت‌افزاری، "
     "یک نردبان LED و برچسب چرخیده — دقیقاً مثل یک رَک استودیو.", "rack", 1),
    ("v3-06-black-glass", "BLACK GLASS", "شیشهٔ سیاه",
     "سطح شیشهٔ مشکی پولیش‌شده؛ پرتره در نیمهٔ بالا و انعکاس محوِ آن در نیمهٔ پایین، "
     "جدا شده با یک خط افق قرمز موی‌باریک. پنل‌های شیشه‌ای شفاف روی انعکاس.", "grid", 2),
    ("v3-07-halftone-terminal", "HALFTONE TERMINAL", "هفت‌رنگ چاپی",
     "پرتره با شبکهٔ نقطه‌ای درشت (هفت‌رنگ/دیزر) به دو تُن قرمز و مشکی، مثل یک پلیت چاپی. "
     "کنارش هفت ستون فنی با اسلایدرهای عمودی و خوانش عددی.", "cols", 1),
    ("v3-08-diagonal-cut", "DIAGONAL CUT", "برش مورب",
     "پنجره با یک مورب درخشان قرمز برش خورده. مثلث پایین‌چپ پرترهٔ نزدیک با نور از پایین؛ "
     "مثلث بالا‌راست شبکهٔ کاشی‌های ماژول.", "grid", 2),
    ("v3-09-cinematic", "CINEMATIC", "سینمایی ۲۱:۹",
     "قاب پهن سینمایی. پرتره در یک پنل عمودی سمت راست با نور لبهٔ قرمز؛ "
     "دو سوم چپ یک کنسول آرام با ناب‌های بزرگ و مترهای LED پهن.", "grid", 2),
    ("v3-10-monolith-arch", "MONOLITH ARCH", "طاق یکپارچه",
     "رادیکال‌ترین در سکوت بصری. پرتره در یک طاق/روزنهٔ بلند و باریک در بالا، بسیار تاریک. "
     "زیرش هفت دیال نازک مثل توقف‌های ارگ در یک قوس متقارن، و فضای خالی عظیم.", "cols", 1),
]

MODULES = ["TONE", "DYNAMICS", "COLOUR", "SPACE", "IMAGE", "VOCAL", "MASTER"]

def b64(name):
    p = os.path.join("assets/gui", name + ".png")
    return "data:image/png;base64," + base64.b64encode(open(p, "rb").read()).decode()

rows = ""
for i, (num, en, fa, desc, kind, cols) in enumerate(OPTS, 1):
    rows += f"""
<section class="card">
  <div class="head">
    <div class="n">{i:02d}</div>
    <div><h2>{en}</h2><div class="fa">{fa}</div></div>
  </div>
  <img src="{b64(num)}" alt="{en}" loading="lazy">
  <p class="desc">{desc}</p>
</section>"""

HTML = f"""<!DOCTYPE html>
<html lang="fa" dir="rtl"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MH SIGNATURE — ۱۰ طرح با پرترهٔ میلاد</title>
<style>
 :root {{ --bg:#07080a; --card:#0d0f13; --line:#1d222a; --ink:#eef1f5;
          --mut:#8a929e; --red:#d42f38; --red2:#8f1b22; }}
 * {{ box-sizing:border-box; }}
 body {{ margin:0; color:var(--ink); line-height:1.85; font-size:15px;
   font-family:"Vazirmatn","IRANSans","Segoe UI",Tahoma,sans-serif;
   background: radial-gradient(1100px 520px at 76% -8%, #1b0e12 0%, transparent 62%),
               radial-gradient(900px 460px at 4% 106%, #241014 0%, transparent 62%), var(--bg); }}
 .wrap {{ max-width:1200px; margin:0 auto; padding:46px 20px 100px; }}
 header {{ text-align:center; border-bottom:1px solid var(--line); padding-bottom:28px; }}
 .logo {{ font-size:40px; letter-spacing:.44em; text-indent:.44em; font-weight:200;
   margin:0; text-transform:uppercase; }}
 .by {{ color:var(--mut); font-size:12.5px; letter-spacing:.28em; text-transform:uppercase; }}
 .lead {{ color:#b7bfca; max-width:760px; margin:20px auto 0; font-size:14.5px; }}
 .tools {{ display:flex; gap:8px; justify-content:center; flex-wrap:wrap; margin-top:22px; }}
 .mods {{ display:flex; gap:7px; flex-wrap:wrap; justify-content:center; }}
 .mod {{ font-family:ui-monospace,monospace; font-size:11.5px; letter-spacing:.13em;
   border:1px solid var(--line); border-radius:20px; padding:4px 12px; color:#b6bec9;
   background:#0f1216; }}
 .card {{ background:var(--card); border:1px solid var(--line); border-radius:16px;
   padding:18px; margin:26px 0; }}
 .card img {{ width:100%; display:block; border-radius:10px; border:1px solid #15191f; }}
 .head {{ display:flex; align-items:center; gap:15px; margin-bottom:14px; }}
 .n {{ font-size:24px; font-weight:200; color:var(--red); font-family:ui-monospace,monospace;
   border:1px solid var(--red2); border-radius:10px; padding:1px 11px; }}
 h2 {{ margin:0; font-size:18px; letter-spacing:.18em; font-weight:500;
   text-transform:uppercase; }}
 .fa {{ color:var(--mut); font-size:13px; }}
 .desc {{ color:#c3cbd6; font-size:14px; margin:13px 0 2px; }}
 .warn {{ margin:34px 0 0; padding:16px 18px; border:1px solid #4a2a2c; border-radius:12px;
   background:#150d0f; font-size:13.5px; color:#dcb9bb; }}
 .warn b {{ color:#e88b8f; }}
 .swatches {{ display:flex; gap:10px; justify-content:center; margin-top:20px; flex-wrap:wrap; }}
 .sw {{ display:flex; align-items:center; gap:7px; font-size:11.5px; color:var(--mut);
   border:1px solid var(--line); border-radius:20px; padding:4px 12px; direction:ltr;
   font-family:ui-monospace,monospace; }}
 .dot {{ width:13px; height:13px; border-radius:50%; border:1px solid #33383f; }}
 footer {{ margin-top:40px; padding-top:18px; border-top:1px solid var(--line);
   color:var(--mut); font-size:12.5px; text-align:center; }}
 code {{ direction:ltr; display:inline-block; background:#181c22; padding:1px 7px;
   border-radius:5px; font-size:12.5px; }}
</style></head><body><div class="wrap">

<header>
  <h1 class="logo">MH Signature</h1>
  <div class="by">by Milad Hashemi</div>
  <p class="lead">ده جهت طراحی دیگر — همه با پرترهٔ میلاد هاشمی به‌عنوان جزئی از خودِ رابط،
  همه با تم نوآر مشکی و قرمز، بدون هیچ طلایی و زردی.</p>
  <div class="tools"><div class="mods">
    {''.join(f'<span class="mod">{m}</span>' for m in MODULES)}
  </div></div>
  <div class="swatches">
    <div class="sw"><span class="dot" style="background:#07080a"></span>#07080A</div>
    <div class="sw"><span class="dot" style="background:#0d0f13"></span>#0D0F13</div>
    <div class="sw"><span class="dot" style="background:#d42f38"></span>#D42F38</div>
    <div class="sw"><span class="dot" style="background:#8f1b22"></span>#8F1B22</div>
    <div class="sw"><span class="dot" style="background:#eef1f5"></span>#EEF1F5</div>
  </div>
</header>

{rows}

<div class="warn">
  <b>دو نکته که باید بدانید:</b>
  در طرح <b>۰۱</b> برچسب «VOCAL» دو بار تکرار شده و یک سلول خالی مانده؛ در طرح <b>۰۴</b> هم
  «COLOUR» و «VOCAL» تکراری شده و تعداد پادها ۹ شده. بقیهٔ هشت طرح دقیقاً هفت ماژول درست دارند.
  سقف تولید تصویر این نوبت پر شد — بگویید تا در پیام بعدی همین دو را اصلاح کنم.
  <br><br>
  همچنین در طرح <b>۰۸</b> یک کاشی خالی اضافه هست که در نسخهٔ نهایی حذف می‌شود.
</div>

<footer>
  MH SIGNATURE · ۱۰ طرح · پرتره: <code>assets/brand/milad-portrait-noir.png</code><br>
  تصاویر: <code>assets/gui/v3-*.png</code>
</footer>
</div></body></html>"""

open("assets/gui/gallery-v3.html", "w").write(HTML)
print("ok", os.path.getsize("assets/gui/gallery-v3.html") // 1024, "KB")
