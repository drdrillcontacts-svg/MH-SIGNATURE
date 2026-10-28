#!/usr/bin/env python3
"""MH SIGNATURE — v7: the reference layout rebuilt in glass (5 executions)."""
import base64, os

OPTS = [
    ("v7-01-glass-clear", "GLASS CLEAR", "شیشهٔ شفاف تیره",
     "وفادارترین اجرا به تصویر مرجع: همان چیدمان دقیق با همان شدت تاریکی، ولی همهٔ پنل‌ها "
     "شیشهٔ مشکی پولیش‌شده با هایلایت تیز روی لبه‌ها هستند. کارت‌ها، ناب‌ها و مقادیر "
     "دقیقاً سر جای خودشان‌اند.",
     ["بیشترین شباهت به مرجع", "همهٔ ناب‌ها برچسب و مقدار دارند", "شیشه در حد «لمس»، نه اغراق"],
     "اگر می‌خواهید نسخهٔ نهایی همان چیزی باشد که طراحی کرده‌اید، فقط با جنس شیشه."),
    ("v7-02-glass-smoked", "GLASS SMOKED", "شیشهٔ دودی",
     "شیشهٔ دودی ضخیم با عمق و نور قرمز حبس‌شده درون شیشه. کنتراست نرم‌تر و لایه‌های "
     "شیشه‌ای واضح‌تر از طرح شفاف.",
     ["عمق و لایه‌بندی چشمگیر", "حس گران‌قیمت و سنگین", "نور قرمز از درون متریال می‌آید"],
     "اگر می‌خواهید پلاگین حس یک وسیلهٔ لوکس و سنگین داشته باشد."),
    ("v7-03-glass-crystal", "GLASS CRYSTAL", "شیشهٔ تراش‌خورده",
     "پنل‌ها و ناب‌ها مثل بلور سیاه تراش‌خورده‌اند: ضخامت دیده می‌شود، لبه‌ها منشوری‌اند و "
     "رفرکشن ملایمی دارند. ناب اصلی به یک نگین چندوجهی تبدیل شده.",
     ["متمایزترین جنس در میان پنج طرح", "ناب اصلی به قطعهٔ شاخص تبدیل می‌شود", "حس جواهری و دقت"],
     "اگر می‌خواهید پلاگین هیچ‌شبیه رقبا نباشد و «نگین» داشته باشد."),
    ("v7-04-glass-mirror", "GLASS MIRROR", "شیشهٔ آینه‌ای",
     "سطح شیشهٔ آینه‌ای مشکی با رگه‌های نور قرمز که مورب روی سطح می‌لغزند و رفلکس عمودی "
     "واضح. کنترل‌ها کمی بالای شیشه شناورند با سایهٔ تماس نرم.",
     ["تمیزترین و مدرن‌ترین اجرا", "رگه‌های قرمز واقعاً «حرکت» را حس می‌دهند", "خوانایی بالا"],
     "اگر می‌خواهید ترکیب شیشه و رگهٔ قرمز در حد اعلا دیده شود."),
    ("v7-05-glass-frosted", "GLASS FROSTED", "شیشهٔ مات (فوگ)",
     "شیشهٔ مات حکاکی‌شده با دانه‌بندی ریز و درخشش قرمز ملایم از لبه‌ها. ناب‌ها شیشهٔ تیرهٔ "
     "شفاف روی سطح مات نشسته‌اند — کنتراست دو جنس.",
     ["کم‌بازتاب‌ترین و راحت‌ترین برای کار طولانی", "ترکیب دو جنس شیشه در یک قاب", "حس استودیویی آرام"],
     "اگر می‌خواهید پلاگین ساعت‌ها روی چشم سنگینی نکند."),
]

def b64(n):
    return "data:image/png;base64," + base64.b64encode(
        open(os.path.join("assets/gui", n + ".png"), "rb").read()).decode()

cards = ""
for i, (num, en, fa, desc, bullets, best) in enumerate(OPTS, 1):
    bl = "".join(f"<li>{b}</li>" for b in bullets)
    cards += f"""
<section class="card">
  <div class="head">
    <div class="n">{i:02d}</div>
    <div><h2>{en}</h2><div class="fa">{fa}</div></div>
    <div class="flagbox"><span class="ok">۷ ماژول · همه برچسب‌دار</span></div>
  </div>
  <img src="{b64(num)}" alt="{en}" loading="lazy">
  <p class="desc">{desc}</p>
  <ul class="bl">{bl}</ul>
  <p class="best"><span>پیشنهاد برای:</span> {best}</p>
</section>"""

HTML = f"""<!DOCTYPE html>
<html lang="fa" dir="rtl"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MH SIGNATURE — همان چیدمان، این بار شیشه‌ای</title>
<style>
 :root {{ --bg:#050607; --card:#0b0d11; --line:#1a1e24; --ink:#eef1f5;
          --mut:#8a929e; --red:#d42f38; --red2:#8f1b22; }}
 * {{ box-sizing:border-box; }}
 body {{ margin:0; background:var(--bg); color:var(--ink); font-size:14.5px;
   line-height:1.85; font-family:"Vazirmatn","IRANSans","Segoe UI",Tahoma,sans-serif;
   background-image:radial-gradient(900px 420px at 84% -6%, #180c10 0%, transparent 60%),
                    radial-gradient(760px 360px at 2% 104%, #1b0c0f 0%, transparent 60%); }}
 .wrap {{ max-width:1240px; margin:0 auto; padding:44px 18px 96px; }}
 header {{ text-align:center; border-bottom:1px solid var(--line); padding-bottom:26px; }}
 .logo {{ font-size:38px; letter-spacing:.44em; text-indent:.44em; font-weight:200;
   margin:0; text-transform:uppercase; }}
 .by {{ color:var(--mut); font-size:12px; letter-spacing:.26em; text-transform:uppercase; }}
 .lead {{ color:#b7bfca; max-width:840px; margin:18px auto 0; font-size:14.5px; }}
 .card {{ background:var(--card); border:1px solid var(--line); border-radius:16px;
   padding:18px; margin:22px 0; }}
 .card img {{ width:100%; display:block; border-radius:10px; border:1px solid #12161b; }}
 .head {{ display:flex; align-items:center; gap:14px; margin-bottom:14px; flex-wrap:wrap; }}
 .n {{ font-size:23px; font-weight:200; color:var(--red); font-family:ui-monospace,monospace;
   border:1px solid var(--red2); border-radius:10px; padding:1px 11px; }}
 h2 {{ margin:0; font-size:17.5px; letter-spacing:.18em; font-weight:500;
   text-transform:uppercase; }}
 .fa {{ color:var(--mut); font-size:13px; }}
 .flagbox {{ margin-right:auto; }}
 .ok {{ font-size:11.5px; border-radius:20px; padding:3px 10px;
   background:#0f1614; border:1px solid #22423a; color:#77bda8; }}
 .desc {{ color:#c3cbd6; font-size:14px; margin:13px 0 8px; }}
 ul.bl {{ margin:4px 0 12px; padding-right:20px; color:#aab3bf; font-size:13.4px; }}
 .best {{ font-size:13.4px; background:#150c0e; border-right:2px solid var(--red);
   border-radius:8px; padding:9px 12px; color:#d3d9e1; }}
 .best span {{ color:var(--red); font-weight:600; }}
 .note {{ border-right:3px solid var(--red); background:#140c0e; padding:13px 15px;
   border-radius:9px; font-size:13.5px; color:#d6b7b9; margin:20px 0 0; }}
 footer {{ margin-top:36px; padding-top:16px; border-top:1px solid var(--line);
   color:var(--mut); font-size:12.3px; text-align:center; }}
 code {{ direction:ltr; display:inline-block; background:#171b21; padding:1px 6px;
   border-radius:5px; font-size:12px; }}
</style></head><body><div class="wrap">

<header>
  <h1 class="logo">MH Signature</h1>
  <div class="by">by Milad Hashemi · reference layout · glass edition</div>
  <p class="lead">همان چیدمان مرجع شما — ستون پرتره در چپ، موتور امضا با ناب بزرگ،
  آنالایزر طیف در راست، EQ و COMPRESSOR و SATURATION در بالا، ریورب و دِلی و مدولاسیون و
  ایمجر در پایین، مترهای INPUT/OUTPUT در دو لبه — با همان شدت مشکی و قرمز،
  این بار در <b>پنج جنس مختلف شیشه</b>.</p>
</header>

{cards}

<div class="note">
  در همهٔ پنج طرح، هر ناب برچسب و مقدار دارد: CHARACTER 62 · DENSITY 48 · SPACE 70 · WIDTH 56،
  و در کارت‌ها LOW 0.0dB · LOW MID −2.4dB · THR −18.0dB · ATTACK 10ms · DECAY 2.4s ·
  FEEDBACK 36% · RATE 0.30Hz · WIDTH 132% و بقیه.
  بگویید کدام را می‌خواهید تا نسخهٔ نهایی همان را در ابعاد دقیق (مثلاً ۱۴۰۰×۷۶۰) و با
  گرید کامل برای پیاده‌سازی تحویل بدهم.
</div>

<footer>
  MH SIGNATURE · glass edition · ۵ اجرا · تصاویر: <code>assets/gui/v7-*.png</code><br>
  بدون طلایی، بدون زرد · پرتره: <code>assets/brand/milad-portrait-noir.png</code>
</footer>
</div></body></html>"""

open("assets/gui/gallery-v7.html", "w").write(HTML)
print("ok", os.path.getsize("assets/gui/gallery-v7.html") // 1024, "KB")
