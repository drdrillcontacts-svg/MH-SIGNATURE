#!/usr/bin/env python3
"""Build the 5-option GUI gallery for MH SIGNATURE."""
import base64, os

OPTS = [
    ("01", "NOIR CONSOLE", "چassis آنالوگ نوآر",
     "پنل مشکی مات با بافت برس‌خورده و پیچ‌های گوشه — شبیه یک یونیت رَک واقعی. "
     "VU متر آنالوگ در سمت چپ، ماژول‌ها به‌صورت ستون‌های عمودی، اسلایدر Master و کلید Power.",
     ["بیشترین حس «سخت‌افزار واقعی»", "خوانایی عالی برای تنظیم سریع",
      "کاملاً مینیمال: هیچ عدد و نوشتهٔ اضافه‌ای ندارد"],
     "اگر می‌خواهید پلاگین حس یک Low-end آنالوگ لوکس بدهد و کاربر بدون آموزش بفهمد چطور کار می‌کند."),
    ("02", "SIGNAL FLOW", "زنجیرهٔ سیگنال",
     "شش کارت شیشه‌ای تیره روی زمینهٔ مشکی، با خطوط قرمز مسیر سیگنال بین‌شان. "
     "رینگ بزرگ Gain Reduction در سمت راست و آنالایزر LUFS/PLR در پایین.",
     ["روایت‌گرترین طرح: زنجیرهٔ میکس را literally نشان می‌دهد", "بیشترین کاربرد آموزشی برای هنرجو",
      "حس دیجیتال مدرن و شیشه‌ای"],
     "اگر هدف این است که پلاگین هم ابزار باشد هم «کلاس درس سلیقهٔ میلاد»."),
    ("03", "SPECTRUM NOIR", "نمایشگر طیف",
     "کل صفحه وقف یک نمایشگر EQ عظیم با منحنی قرمز گرم روی گرید مشکی، "
     "و منحنی مرجع نقطه‌چین خاکستری برای مقایسه. چهار پاد کنترلی گرداگرد آن.",
     ["تمرکز کامل روی «رنگ و جنس» — قلب امضای میلاد", "منحنی مرجع، تطبیق با ۲۲ اثر را ممکن می‌کند",
      "زیباترین طرح در اسکرین‌شات و بازاریابی"],
     "اگر می‌خواهید محصول حول «SIGNATURE MATCH» ساخته شود."),
    ("04", "SWISS MINIMAL", "سوئیسی مینیمال",
     "منفی‌فضای تقریباً کامل مشکی، فقط خطوط موی‌باریک قرمز و یک گرید هندسی دقیق. "
     "چهار دیال بزرگ با حلقهٔ نازک، یک نوار VU و یک موج تک‌خطی.",
     ["ناب‌ترین مینیمالیسم، حس گالری/پوستر", "سبک‌ترین و ارزان‌ترین برای رندر و مقیاس‌پذیری",
      "تایپوگرافی بزرگ و مغرور"],
     "اگر می‌خواهید محصول از رقبا کاملاً متمایز و «طراحانه» دیده شود؛ برای کاربر حرفه‌ای."),
    ("05", "HYBRID DIGITAL", "ترکیب دیجیتال/آنالوگ",
     "شاسی فلزی ذغالی با شیارهای عمودی + نمایشگر اسیلوسکوپ پهن. "
     "ناب‌های نارنجی‌قرمز با گرپ آج‌دار، دو سوییچ فلزی و نوار نمایشگر متنی قرمز.",
     ["دقیقاً همان «ترکیب دیجیتال و آنالوگ» که خواستید", "بیشترین جزئیات و حس تاچ",
      "تعادل خوب بین حس سخت‌افزار و مدرنیته"],
     "اگر می‌خواهید هم حس آنالوگ داشته باشد هم مدرن، با بیشترین عمق بصری."),
]

def img(opt):
    num = opt[0]
    f = [p for p in os.listdir("assets/gui") if p.startswith("gui-" + num)][0]
    b = base64.b64encode(open("assets/gui/" + f, "rb").read()).decode()
    return f"data:image/png;base64,{b}"

cards = ""
for i, o in enumerate(OPTS):
    num, en, fa, desc, bullets, best = o
    bullets_html = "".join(f"<li>{b}</li>" for b in bullets)
    cards += f"""
<section class="card" id="opt-{num}">
  <div class="head">
    <div class="n">۰{int(num)}</div>
    <div>
      <h2>{en}</h2>
      <div class="fa">{fa}</div>
    </div>
  </div>
  <img src="{img(o)}" alt="GUI option {num}">
  <p class="desc">{desc}</p>
  <ul class="bl">{bullets_html}</ul>
  <p class="best"><span>پیشنهاد برای:</span> {best}</p>
</section>"""

HTML = f"""<!DOCTYPE html>
<html lang="fa" dir="rtl">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MH SIGNATURE — ۵ طرح رابط کاربری</title>
<style>
  :root {{ --bg:#07080a; --card:#101216; --line:#22262d; --ink:#eceff4;
           --mut:#8a929e; --red:#d42f38; --red2:#8f1b22; }}
  * {{ box-sizing:border-box; }}
  body {{ margin:0; background:
      radial-gradient(1200px 600px at 70% -10%, #181014 0%, transparent 60%),
      radial-gradient(900px 500px at 10% 110%, #2a0d11 0%, transparent 60%), var(--bg);
      color:var(--ink); font-family:"Vazirmatn","IRANSans","Segoe UI",Tahoma,sans-serif;
      line-height:1.9; font-size:15px; }}
  .wrap {{ max-width:1180px; margin:0 auto; padding:44px 20px 90px; }}
  header {{ text-align:center; padding-bottom:30px; border-bottom:1px solid var(--line); }}
  .logo {{ font-size:38px; letter-spacing:.42em; font-weight:200; margin:0;
           text-transform:uppercase; text-indent:.42em; }}
  .by {{ color:var(--mut); font-size:13px; letter-spacing:.24em;
         margin-top:6px; text-transform:uppercase; }}
  .lead {{ color:#b9c1cc; max-width:720px; margin:20px auto 0; font-size:14.5px; }}
  .swatches {{ display:flex; gap:10px; justify-content:center; margin-top:22px; flex-wrap:wrap; }}
  .sw {{ display:flex; align-items:center; gap:7px; font-size:11.5px; color:var(--mut);
         border:1px solid var(--line); border-radius:20px; padding:4px 12px;
         direction:ltr; font-family:ui-monospace,monospace; }}
  .dot {{ width:13px; height:13px; border-radius:50%; border:1px solid #33383f; }}
  .card {{ background:var(--card); border:1px solid var(--line); border-radius:16px;
           padding:20px; margin:26px 0; }}
  .card img {{ width:100%; display:block; border-radius:10px; border:1px solid #1a1e24; }}
  .head {{ display:flex; align-items:center; gap:16px; margin-bottom:16px; }}
  .n {{ font-size:30px; font-weight:200; color:var(--red); direction:ltr;
        font-family:ui-monospace,monospace; border:1px solid var(--red2);
        border-radius:10px; padding:2px 10px; }}
  h2 {{ margin:0; font-size:19px; letter-spacing:.16em; font-weight:500; text-transform:uppercase; }}
  .fa {{ color:var(--mut); font-size:13px; letter-spacing:0; }}
  .desc {{ color:#c3cbd6; font-size:14px; margin:14px 0 8px; }}
  ul.bl {{ margin:6px 0 12px; padding-right:20px; color:#aab3bf; font-size:13.5px; }}
  ul.bl li {{ margin:3px 0; }}
  .best {{ font-size:13.5px; background:#160c0e; border-right:3px solid var(--red);
           border-radius:8px; padding:10px 13px; color:#d3d9e1; }}
  .best span {{ color:var(--red); font-weight:600; }}
  .pick {{ margin:40px 0 10px; padding:22px; border:1px solid var(--red2);
           border-radius:14px; background:linear-gradient(180deg,#130b0d,#0b0c0f); }}
  .pick h3 {{ margin:0 0 10px; color:var(--red); font-size:16px; letter-spacing:.06em; }}
  .pick p {{ margin:6px 0; font-size:14px; color:#c9d0da; }}
  code {{ direction:ltr; display:inline-block; background:#1b1f26; padding:1px 7px;
          border-radius:5px; font-size:12.5px; }}
  footer {{ margin-top:44px; padding-top:18px; border-top:1px solid var(--line);
            color:var(--mut); font-size:12.5px; text-align:center; }}
</style>
</head>
<body><div class="wrap">

<header>
  <h1 class="logo">MH Signature</h1>
  <div class="by">by Milad Hashemi</div>
  <p class="lead">پنج جهت‌گیری بصری برای رابط کاربری پلاگین — همه در تم نوآر مشکی و قرمز، بدون هیچ طلایی و زردی.
  هر پنج طرح از دادهٔ همان ۲۲ میکس استخراج شده‌اند.</p>
  <div class="swatches">
    <div class="sw"><span class="dot" style="background:#07080a"></span>#07080A NOIR</div>
    <div class="sw"><span class="dot" style="background:#101216"></span>#101216 PANEL</div>
    <div class="sw"><span class="dot" style="background:#d42f38"></span>#D42F38 CRIMSON</div>
    <div class="sw"><span class="dot" style="background:#8f1b22"></span>#8F1B22 DEEP RED</div>
    <div class="sw"><span class="dot" style="background:#eceff4"></span>#ECEFF4 INK</div>
  </div>
</header>

{cards}

<div class="pick">
  <h3>یک توصیهٔ صریح</h3>
  <p>اگر باید یکی را انتخاب کنم: <b>۰۵ HYBRID DIGITAL</b> دقیقاً همان چیزی است که خواستید — «ترکیب دیجیتال و آنالوگ» — با
  بیشترین عمق بصری و در عین حال خوانایی. اما برای اینکه پلاگین واقعاً <i>MH SIGNATURE</i> باشد،
  <b>۰۳ SPECTRUM NOIR</b> بهترین انتخاب است: کل هویت محصول حول منحنی امضا و تطبیق با ۲۲ اثر ساخته می‌شود.</p>
  <p>و یک ترکیب پیشنهادی: <b>پوستهٔ ۰۵ (شاسی و مترها) + نمایشگر بزرگ ۰۳ (منحنی امضا) + چیدمان ماژولی ۰۲ (مسیر سیگنال)</b>.
  بگویید کدام را می‌خواهید، یا بگویید ترکیب — نسخهٔ نهایی و دقیق‌تر را می‌سازم.</p>
</div>

<footer>
  MH SIGNATURE · طراحی رابط · ۵ گزینه · فایل‌ها در <code>assets/gui/</code><br>
  تمام طرح‌ها بر پایهٔ تحلیل ۲۲ میکس میلاد هاشمی · بدون طلایی، بدون زرد
</footer>
</div></body></html>
"""

open("assets/gui/gallery.html", "w").write(HTML)
print("ok", os.path.getsize("assets/gui/gallery.html") // 1024, "KB")
