#!/usr/bin/env python3
"""MH SIGNATURE — final glass-mirror finalists with the #D0361C palette."""
import base64, os

OPTS = [
    ("final-glass-mirror-a", "MIRROR · A", "آینه‌ای — اجرای وفادار",
     "همان چیدمان مرجع با رگهٔ نور قرمز که از گوشهٔ بالا‌راست می‌گذرد. "
     "مترهای INPUT و OUTPUT کامل و مدرج، هر هفت کارت با نام و مقدار کامل.",
     ["نزدیک‌ترین به مرجع", "مترها با اعداد dB کامل", "رگهٔ نور چشمگیر ولی مهارشده"],
     "اگر می‌خواهید نسخهٔ نهایی همان مرجع باشد، فقط با قرمز #D0361C."),
    ("final-glass-mirror-b", "MIRROR · B", "آینه‌ای — اجرای پرکنتراست",
     "کنتراست بالاتر و رفلکس واضح‌تر روی لبه‌ها. کارت‌های پایین عمودی‌تر چیده شده‌اند "
     "و هر پارامتر با نام و مقدار در یک ستون خوانده می‌شود.",
     ["خواناترین فهرست پارامترها", "کنتراست عالی برای نمایشگرهای تاریک", "حس گران‌قیمت"],
     "اگر می‌خواهید همهٔ مقادیر در یک نگاه خوانده شوند."),
    ("final-glass-mirror-c", "MIRROR · C", "آینه‌ای — اجرای دراماتیک",
     "رگهٔ قرمز پهن و مورب که از سراسر سطح می‌گذرد و در شیشه بازتاب می‌شود. "
     "پرتره بزرگ‌تر و ناب اصلی با آرک نازک.",
     ["بیشترین حس شیشه و رگهٔ نور", "چشمگیرترین در اسکرین‌شات", "کمی شلوغ‌تر از بقیه"],
     "اگر می‌خواهید طرح در نگاه اول «وای» بدهد."),
    ("final-glass-mirror-d", "MIRROR · D", "آینه‌ای — اجرای متعادل ★",
     "تعادل کامل: فاصله‌گذاری‌های باز، تایپوگرافی ریز و دقیق، رگهٔ نور ملایم و رفلکس‌های تمیز. "
     "نام و مقدار هر ناب کاملاً خوانا، و بلوک GAIN-REDUCTION در کمپرسور.",
     ["تمیزترین و حرفه‌ای‌ترین", "بیشترین فضای تنفس", "آمادهٔ پیاده‌سازی؛ کمترین ریسک UI"],
     "پیشنهاد من برای نسخهٔ نهایی."),
]

def b64(n):
    return "data:image/png;base64," + base64.b64encode(
        open(os.path.join("assets/gui", n + ".png"), "rb").read()).decode()

cards = ""
for i, (num, en, fa, desc, bullets, best) in enumerate(OPTS, 1):
    bl = "".join(f"<li>{b}</li>" for b in bullets)
    star = ' · <b>انتخاب پیشنهادی</b>' if en.endswith("D") else ""
    cards += f"""
<section class="card{' hero' if en.endswith('D') else ''}">
  <div class="head">
    <div class="n">{i:02d}</div>
    <div><h2>{en}</h2><div class="fa">{fa}{star}</div></div>
    <div class="flagbox"><span class="ok">#D0361C · شیشهٔ آینه‌ای</span></div>
  </div>
  <img src="{b64(num)}" alt="{en}" loading="lazy">
  <p class="desc">{desc}</p>
  <ul class="bl">{bl}</ul>
  <p class="best"><span>پیشنهاد برای:</span> {best}</p>
</section>"""

HTML = f"""<!DOCTYPE html>
<html lang="fa" dir="rtl"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MH SIGNATURE — GLASS MIRROR · قرمز #D0361C</title>
<style>
 :root {{ --bg:#050607; --card:#0a0c0f; --line:#181c22; --ink:#EEF1F5;
          --mut:#8A929E; --acc:#D0361C; --acc-deep:#8F2412; --acc-soft:#F2745A; }}
 * {{ box-sizing:border-box; }}
 body {{ margin:0; background:var(--bg); color:var(--ink); font-size:14.5px;
   line-height:1.85; font-family:"Vazirmatn","IRANSans","Segoe UI",Tahoma,sans-serif;
   background-image:radial-gradient(900px 420px at 84% -6%, #1d0a06 0%, transparent 62%),
                    radial-gradient(760px 360px at 2% 104%, #220c07 0%, transparent 62%); }}
 .wrap {{ max-width:1240px; margin:0 auto; padding:44px 18px 96px; }}
 header {{ text-align:center; border-bottom:1px solid var(--line); padding-bottom:26px; }}
 .logo {{ font-size:38px; letter-spacing:.44em; text-indent:.44em; font-weight:200;
   margin:0; text-transform:uppercase; }}
 .by {{ color:var(--mut); font-size:12px; letter-spacing:.26em; text-transform:uppercase; }}
 .lead {{ color:#b9c0c9; max-width:850px; margin:18px auto 0; font-size:14.5px; }}
 .sw {{ display:flex; gap:9px; justify-content:center; flex-wrap:wrap; margin-top:22px; }}
 .chip {{ display:flex; align-items:center; gap:7px; font-size:11.5px; color:var(--mut);
   border:1px solid var(--line); border-radius:20px; padding:4px 12px; direction:ltr;
   font-family:ui-monospace,monospace; }}
 .dot {{ width:13px; height:13px; border-radius:50%; border:1px solid #2a2f36; }}
 .card {{ background:var(--card); border:1px solid var(--line); border-radius:16px;
   padding:18px; margin:22px 0; }}
 .card.hero {{ border-color:var(--acc-deep); box-shadow:0 0 0 1px rgba(208,54,28,.18),
   radial-gradient(600px 200px at 50% 0%, rgba(208,54,28,.10), transparent 70%); }}
 .card img {{ width:100%; display:block; border-radius:10px; border:1px solid #10141a; }}
 .head {{ display:flex; align-items:center; gap:14px; margin-bottom:14px; flex-wrap:wrap; }}
 .n {{ font-size:23px; font-weight:200; color:var(--acc); font-family:ui-monospace,monospace;
   border:1px solid var(--acc-deep); border-radius:10px; padding:1px 11px; }}
 h2 {{ margin:0; font-size:17.5px; letter-spacing:.18em; font-weight:500;
   text-transform:uppercase; }}
 .fa {{ color:var(--mut); font-size:13px; }}
 .fa b {{ color:var(--acc-soft); }}
 .flagbox {{ margin-right:auto; }}
 .ok {{ font-size:11.5px; border-radius:20px; padding:3px 10px;
   background:#140806; border:1px solid #4a1c10; color:var(--acc-soft);
   font-family:ui-monospace,monospace; direction:ltr; }}
 .desc {{ color:#c3cbd6; font-size:14px; margin:13px 0 8px; }}
 ul.bl {{ margin:4px 0 12px; padding-right:20px; color:#aab3bf; font-size:13.4px; }}
 .best {{ font-size:13.4px; background:#150a07; border-right:2px solid var(--acc);
   border-radius:8px; padding:9px 12px; color:#d6dbe1; }}
 .best span {{ color:var(--acc-soft); font-weight:600; }}
 .note {{ border-right:3px solid var(--acc); background:#140a07; padding:13px 15px;
   border-radius:9px; font-size:13.5px; color:#dcc0b7; margin:20px 0 0; }}
 footer {{ margin-top:36px; padding-top:16px; border-top:1px solid var(--line);
   color:var(--mut); font-size:12.3px; text-align:center; }}
 code {{ direction:ltr; display:inline-block; background:#15181d; padding:1px 6px;
   border-radius:5px; font-size:12px; }}
</style></head><body><div class="wrap">

<header>
  <h1 class="logo">MH Signature</h1>
  <div class="by">by Milad Hashemi · glass mirror · #D0361C</div>
  <p class="lead">GLASS MIRROR انتخاب شد. رنگ قرمز از <code>#D42F38</code>
  به <code>#D0361C</code> تغییر کرد — یک وِرمیلیون آجری پرمایه که پریدگی و حس رزگونگی قبلی را ندارد.
  پرتره هم با همان هویت رنگی بازسازی شد. چهار اجرا از همین طرح:</p>
  <div class="sw">
    <div class="chip"><span class="dot" style="background:#050607"></span>#050607 NOIR</div>
    <div class="chip"><span class="dot" style="background:#101216"></span>#101216 PANEL</div>
    <div class="chip"><span class="dot" style="background:#d0361c"></span>#D0361C ACCENT</div>
    <div class="chip"><span class="dot" style="background:#8f2412"></span>#8F2412 DEEP</div>
    <div class="chip"><span class="dot" style="background:#f2745a"></span>#F2745A SOFT</div>
  </div>
</header>

{cards}

<div class="note">
  هر چهار اجرا از پالت یکسان <code>#D0361C</code> استفاده می‌کنند — تفاوت‌شان فقط در
  شدت رگهٔ نور، کنتراست شیشه و فاصله‌گذاری است. توکن‌های رنگ برای توسعه در
  <code>design/tokens.json</code> ثبت شد.
</div>

<footer>
  MH SIGNATURE · GLASS MIRROR · ۴ اجرا · تصاویر: <code>assets/gui/final-*.png</code><br>
  پرتره: <code>assets/brand/milad-portrait-vermilion.png</code> · بدون طلایی، بدون زرد
</footer>
</div></body></html>"""

open("assets/gui/gallery-final.html", "w").write(HTML)
print("ok", os.path.getsize("assets/gui/gallery-final.html") // 1024, "KB")
