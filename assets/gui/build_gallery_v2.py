#!/usr/bin/env python3
"""MH SIGNATURE — v2 gallery: 5 GUIs, each with all 7 modules + his portrait."""
import base64, os

OPTS = [
    ("v2-01-portrait-panel", "PORTRAIT PANEL", "پرتره + شبکهٔ ماژول",
     "چهرهٔ میلاد در یک پنجرهٔ عمودی سمت چپ، از قرمز به سیاه محو می‌شود. "
     "سمت راست: هفت ماژول در شبکهٔ ۴+۳ — TONE / DYNAMICS / COLOUR / SPACE در ردیف بالا و "
     "IMAGE / VOCAL / MASTER در ردیف پایین. هر ماژول ناب‌های مات مشکی با یک خط نشانگر قرمز و نوار لِوِل.",
     ["هر ۷ ماژول با فاصلهٔ برابر و خوانایی کامل", "چهره در نقش «پلاک هویت» محصول",
      "ترین و مرتب‌ترین برای توسعهٔ واقعی UI"],
     "اگر می‌خواهید پلاگین حس یک محصول آماده و حرفه‌ای بدهد و کدنویسی UI ساده باشد."),

    ("v2-02-medallion-rack", "MEDALLION RACK", "مدالیون مرکزی",
     "پرتره در یک مدالیون گرد مثل لنز یا یک VU متر غول‌آسا، دقیقاً در مرکز. "
     "سه ماژول در چپ (TONE / DYNAMICS / COLOUR)، سه در راست (SPACE / IMAGE / VOCAL) و "
     "MASTER با اسلایدر افقی زیر مدالیون. مترهای LED قرمز دو طرف.",
     ["تقارن کامل و حس سخت‌افزار رَک", "چهره قلب بصری پلاگین است، نه یک تصویر کنار",
      "بیشترین حس «لوکس و گران»"],
     "اگر می‌خواهید محصول در نگاه اول شما را یادگاری از میلاد بداند و لوکس دیده شود."),

    ("v2-03-chain-strip", "CHAIN STRIP", "نوار زنجیره",
     "پرتره به‌صورت یک نوار عمودی باریک در لبهٔ چپ، محو در سیاه. "
     "هفت کارت شیشه‌ای در یک ردیف با خطوط قرمزِ مسیر سیگنال بین‌شان، "
     "و آنالایزر LUFS در پایین.",
     ["زنجیرهٔ سیگنال را روایت می‌کند: ترتیب واقعی پردازش", "سبک و مدرن، شبیه ابزار حرفه‌ای",
      "پرتره بدون شلوغی در طراحی حل شده"],
     "اگر می‌خواهید ترتیب زنجیره در خود رابط خوانده شود و پلاگین «ابزار» باشد."),

    ("v2-04-swiss-grid", "SWISS GRID", "گرید سوئیسی",
     "کل رابط یک گرید دقیق با خطوط موی‌باریک قرمز. پرتره یک کاشی مربع درون گرید است — "
     "مثل عکس یک پوستر معماری، نه تصویر قهرمان. هفت سلول با نام ماژول‌ها و عناصر مینیمال.",
     ["ناب‌ترین مینیمالیسم و بیشترین تمایز از رقبا", "تایپوگرافی بزرگ و مغرور",
      "پرتره بخشی از ساختار است، نه دکور"],
     "اگر می‌خواهید محصول برندهای مستقل و «طراحانه» دیده شود."),

    ("v2-05-etched-hybrid", "ETCHED HYBRID", "پرترهٔ حک‌شده",
     "پرتره با کنتراست کم، مثل چاپ سیلک روی بدنهٔ فلزی، پشت کنترل‌ها نیمه‌پیدا است — "
     "و از پشت شیشهٔ اسیلوسکوپ هم دیده می‌شود. شاسی ذغالی برس‌خورده، دو سوییچ فلزی، "
     "هفت گروه کنترل در یک ردیف و مترهای LED در پایین.",
     ["قوی‌ترین بیان «ترکیب دیجیتال و آنالوگ»", "چهره درون طراحی حک شده، نه چسبانده",
      "بیشترین عمق و حس لمس‌پذیری"],
     "اگر می‌خواهید پلاگین واقعاً شبیه یک وسیلهٔ فیزیکی دست‌ساز باشد."),
]

MODULES = [
    ("TONE", "EQ شیب‌دار + شلف بم + فروکاستن Presence"),
    ("DYNAMICS", "کمپرسور باس/وکال + سایدچین قفل‌شده روی تمپو"),
    ("COLOUR", "سچوریشن آنالوگ + کلیپر نرم + HF rolloff"),
    ("SPACE", "ریورب تاریک + دِلی داک‌شده، فیلترشده در لوباند"),
    ("IMAGE", "مونو زیر ۹۰Hz + پهنای باندی Mid/Air"),
    ("VOCAL", "دِاِس + حضور + دابلینگ"),
    ("MASTER", "لیمیت + هدف LUFS + پیک واقعی"),
]

def b64(name):
    p = os.path.join("assets/gui", name + ".png")
    return "data:image/png;base64," + base64.b64encode(open(p, "rb").read()).decode()

cards = ""
for num, en, fa, desc, bullets, best in OPTS:
    bl = "".join(f"<li>{b}</li>" for b in bullets)
    cards += f"""
<section class="card" id="{num[:5]}">
  <div class="head"><div class="n">{num[3:5]}</div>
    <div><h2>{en}</h2><div class="fa">{fa}</div></div></div>
  <img src="{b64(num)}" alt="{en}">
  <p class="desc">{desc}</p>
  <ul class="bl">{bl}</ul>
  <p class="best"><span>پیشنهاد برای:</span> {best}</p>
</section>"""

modrows = "".join(f"<tr><td class='m'>{n}</td><td>{d}</td></tr>" for n, d in MODULES)

HTML = f"""<!DOCTYPE html>
<html lang="fa" dir="rtl"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MH SIGNATURE — ۵ طرح با ۷ ماژول و پرتره</title>
<style>
 :root {{ --bg:#07080a; --card:#0e1014; --line:#1e232a; --ink:#eef1f5;
          --mut:#8b939f; --red:#d42f38; --red2:#8f1b22; }}
 * {{ box-sizing:border-box; }}
 body {{ margin:0; color:var(--ink); line-height:1.9; font-size:15px;
   font-family:"Vazirmatn","IRANSans","Segoe UI",Tahoma,sans-serif;
   background: radial-gradient(1100px 520px at 74% -8%, #1a0e12 0%, transparent 62%),
               radial-gradient(900px 460px at 6% 108%, #251014 0%, transparent 62%), var(--bg); }}
 .wrap {{ max-width:1180px; margin:0 auto; padding:46px 20px 96px; }}
 header {{ text-align:center; border-bottom:1px solid var(--line); padding-bottom:30px; }}
 .logo {{ font-size:40px; letter-spacing:.44em; text-indent:.44em; font-weight:200;
   margin:0; text-transform:uppercase; }}
 .by {{ color:var(--mut); font-size:12.5px; letter-spacing:.28em; text-transform:uppercase;
   margin-top:4px; }}
 .lead {{ color:#b7bfca; max-width:740px; margin:20px auto 0; font-size:14.5px; }}
 .mods {{ margin:34px 0 0; background:var(--card); border:1px solid var(--line);
   border-radius:14px; padding:6px 18px 14px; }}
 .mods h3 {{ font-size:13px; letter-spacing:.18em; color:var(--red); text-transform:uppercase;
   margin:14px 0 6px; }}
 table {{ width:100%; border-collapse:collapse; font-size:13.5px; }}
 td {{ padding:7px 10px; border-bottom:1px solid var(--line); }}
 td.m {{ width:150px; letter-spacing:.14em; font-size:12px; color:#dfe5ec;
   font-family:ui-monospace,monospace; }}
 .card {{ background:var(--card); border:1px solid var(--line); border-radius:16px;
   padding:20px; margin:28px 0; }}
 .card img {{ width:100%; display:block; border-radius:10px; border:1px solid #161a20; }}
 .head {{ display:flex; align-items:center; gap:16px; margin-bottom:16px; }}
 .n {{ font-size:26px; font-weight:200; color:var(--red); font-family:ui-monospace,monospace;
   border:1px solid var(--red2); border-radius:10px; padding:2px 12px; }}
 h2 {{ margin:0; font-size:18.5px; letter-spacing:.17em; font-weight:500;
   text-transform:uppercase; }}
 .fa {{ color:var(--mut); font-size:13px; }}
 .desc {{ color:#c3cbd6; font-size:14px; margin:14px 0 8px; }}
 ul.bl {{ margin:6px 0 12px; padding-right:20px; color:#aab3bf; font-size:13.5px; }}
 .best {{ font-size:13.5px; background:#160c0e; border-right:3px solid var(--red);
   border-radius:8px; padding:10px 13px; color:#d3d9e1; }}
 .best span {{ color:var(--red); font-weight:600; }}
 .pick {{ margin:42px 0 0; padding:24px; border:1px solid var(--red2); border-radius:14px;
   background:linear-gradient(180deg,#140b0d,#0a0b0e); }}
 .pick h3 {{ margin:0 0 10px; color:var(--red); font-size:16px; letter-spacing:.06em; }}
 .pick p {{ margin:7px 0; font-size:14px; color:#c9d0da; }}
 .swatches {{ display:flex; gap:10px; justify-content:center; margin-top:22px; flex-wrap:wrap; }}
 .sw {{ display:flex; align-items:center; gap:7px; font-size:11.5px; color:var(--mut);
   border:1px solid var(--line); border-radius:20px; padding:4px 12px; direction:ltr;
   font-family:ui-monospace,monospace; }}
 .dot {{ width:13px; height:13px; border-radius:50%; border:1px solid #33383f; }}
 footer {{ margin-top:46px; padding-top:18px; border-top:1px solid var(--line);
   color:var(--mut); font-size:12.5px; text-align:center; }}
 code {{ direction:ltr; display:inline-block; background:#181c22; padding:1px 7px;
   border-radius:5px; font-size:12.5px; }}
</style></head><body><div class="wrap">

<header>
  <h1 class="logo">MH Signature</h1>
  <div class="by">by Milad Hashemi</div>
  <p class="lead">پنج طرح بازطراحی‌شده — این بار <b>هر هفت ماژول</b> در همهٔ طرح‌ها حضور دارند
  و <b>پرترهٔ میلاد هاشمی جزئی از خودِ طراحی</b> است. تم: نوآر مشکی و قرمز، بدون هیچ طلایی و زردی.</p>
  <div class="swatches">
    <div class="sw"><span class="dot" style="background:#07080a"></span>#07080A</div>
    <div class="sw"><span class="dot" style="background:#0e1014"></span>#0E1014</div>
    <div class="sw"><span class="dot" style="background:#d42f38"></span>#D42F38</div>
    <div class="sw"><span class="dot" style="background:#8f1b22"></span>#8F1B22</div>
    <div class="sw"><span class="dot" style="background:#eef1f5"></span>#EEF1F5</div>
  </div>
</header>

<div class="mods">
  <h3>هفت ماژول در همهٔ طرح‌ها</h3>
  <table>{modrows}</table>
</div>

{cards}

<div class="pick">
  <h3>توصیهٔ من</h3>
  <p><b>۰۵ ETCHED HYBRID</b> بهترین پاسخ به خواستهٔ خودتان است: پرتره درون بدنهٔ فلزی حک شده و
  از پشت شیشهٔ اسیلوسکوپ هم دیده می‌شود — یعنی چهره <i>داخل</i> طراحی است، نه یک تصویر چسبانده‌شده.
  این طرح هم‌زمان حس آنالوگ (شاسی، پیچ، سوییچ) و دیجیتال (نمایشگر، متر LED) را دارد.</p>
  <p><b>۰۲ MEDALLION RACK</b> زیباترین است و چهره قلب بصری محصول می‌شود.</p>
  <p><b>۰۱ PORTRAIT PANEL</b> منظم‌ترین و عملی‌ترین برای توسعهٔ واقعی UI است.</p>
  <p>و یک ترکیب پیشنهادی: <b>شاسی و مترهای ۰۵ + مدالیون ۰۲ در مرکز + نمایشگر منحنی امضا از طرح مرحلهٔ قبل</b>.
  بگویید کدام، یا ترکیب را — نسخهٔ نهایی را با گرید و اندازه‌های دقیق می‌سازم.</p>
</div>

<footer>
  MH SIGNATURE · ۷ ماژول · ۵ طرح · پرتره: <code>assets/brand/milad-portrait-noir.png</code><br>
  تصاویر: <code>assets/gui/v2-*.png</code> · بدون طلایی، بدون زرد
</footer>
</div></body></html>"""

open("assets/gui/gallery-v2.html", "w").write(HTML)
print("ok", os.path.getsize("assets/gui/gallery-v2.html") // 1024, "KB")
