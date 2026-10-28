#!/usr/bin/env python3
"""MH SIGNATURE — v5 gallery, batch 1 (10 designs, 10-module architecture)."""
import base64, os

OPTS = [
    ("v5-01-reference", "REFERENCE", "طرح مرجع — نزدیک‌ترین به تصویر خودتان",
     "ستون پرتره در چپ با امضا و کلمات MUSIC/PEOPLE/EMOTION/ALWAYS · "
     "نوار بالا با مونوگرام MH، تب‌ها و انتخابگر پریست · موتور امضا با ناب بزرگ برس‌خورده · "
     "آنالایزر طیف در راست · هشت کارت ماژول در پایین. عملاً همان چیدمانی که فرستادید، بازسازی‌شده.",
     "پس‌زمینهٔ توسعه؛ کمترین ریسک و بیشترین شباهت به سلیقهٔ خودش.", "۸/۱۰", 0),
    ("v5-02-banner", "CINEMATIC BANNER", "بنر سینمایی بالای صفحه",
     "پرتره در یک نوار پانورامیک باریک در بالای پنجره، محو در سیاه از دو طرف. "
     "زیر آن: تب‌ها، موتور امضا با ناب بزرگ، و ۹ نوار ماژول عمودی.",
     "پرتره دیده می‌شود ولی جای کنترل‌ها را تنگ نمی‌کند.", "۹/۱۰", 0),
    ("v5-03-engine-corona", "ENGINE CORONA", "ناب مرکزی با پرترهٔ حک‌شده",
     "چهرهٔ او داخل خودِ فلزِ ناب غول‌آسا حک شده، با حلقهٔ قرمز مدرج و تیک‌های ریز. "
     "چهار ناب در دو طرف، آنالایزر در بالا‌راست و ۹ کارت ماژول در پایین.",
     "خاص‌ترین ایدهٔ این دسته: پرتره «جزء» مکانیزم است، نه تصویر کنار آن.", "۹/۱۰", 1),
    ("v5-04-left-rail-rows", "LEFT RAIL ROWS", "ریل چپ + ردیف‌های افقی",
     "ریل باریک پرتره در چپ با تیتر عمودی. بقیهٔ رابط ده ردیف افقی است — "
     "یک ردیف برای هر ماژول، با دو ناب، یک اسلایدر و یک گراف کوچک.",
     "خوانایی و نظم مهندسی بی‌نظیر؛ توسعهٔ UI ساده‌ترین.", "۹/۱۰", 0),
    ("v5-05-ghost-engine", "GHOST ENGINE", "پرترهٔ شبح‌وار تمام‌صفحه",
     "پرتره تمام سطح را می‌پوشاند اما تقریباً در سیاه حل شده؛ همهٔ کنترل‌ها روی "
     "پنل‌های شیشه‌ای دودی شناورند.",
     "سینمایی و لایه‌دار؛ ولی برای کار روزمره کمی شلوغ.", "۷/۱۰", 1),
    ("v5-06-mirrored-right", "MIRRORED RIGHT", "قرینه — پرتره در راست",
     "همان چیدمان آشنا اما آینه‌شده: پرتره در ستون راست با تیتر عمودی، "
     "کنسول در چپ با آنالایزر بالا و ۹ کارت در ۳×۳.",
     "تعادل خوب؛ آنالایزر بزرگ بالای کنسول خیلی کاربردی است.", "۸/۱۰", 0),
    ("v5-07-square-tile", "SQUARE TILE", "پرتره در کاشی مربع",
     "پرتره در یک کاشی مربع کوچک پایین‌چپ با قاب موی‌باریک قرمز — متواضع و مهارشده. "
     "بالای آن تیتر و امضای دستی. بقیه: موتور امضا، آنالایزر و ۸ کارت ماژول.",
     "معقول‌ترین تعادل بین حضور چهره و فضای کاری.", "۹/۱۰", 0),
    ("v5-08-aperture-radial", "APERTURE RADIAL", "دیافراگم و پره‌های نور",
     "پرتره در یک دریچهٔ دایره‌ای بزرگ در چپ؛ خطوط نازک قرمز از آن به سمت کنترل‌ها "
     "شعاع می‌دهند و ۹ کارت ماژول در یک قوس پایین چیده شده‌اند.",
     "زیباترین جریان بصری؛ حس یک ابزار اپتیکی دقیق.", "۸/۱۰", 1),
    ("v5-09-fullbleed-right", "FULL-BLEED RIGHT", "پرترهٔ سرریز در لبهٔ راست",
     "پرتره تا لبهٔ راست و خارج از کادر ادامه دارد، بدون قاب. کنسول در دو سوم چپ با "
     "موتور امضا، آنالایزر بزرگ و ۸ کارت ماژول.",
     "قدرتمندترین حضور چهره بدون شلوغی؛ حس پوستر سرمقاله‌ای.", "۹/۱۰", 1),
    ("v5-10-analyzer-lens", "ANALYZER LENS", "پرتره داخل آنالایزر",
     "چهرهٔ او به‌صورت شبح داخل صفحهٔ آنالایزر دیده می‌شود، پشتِ منحنی قرمز — "
     "مثل لنزی تاریک که چهره را نشان می‌دهد. لوگ‌ها و نمایشگر LUFS/TP هم هست.",
     "هوشمندانه‌ترین ادغام: چهره با «ابزار صدا» یکی می‌شود.", "۱۰/۱۰", 1),
]

def b64(n):
    return "data:image/png;base64," + base64.b64encode(
        open(os.path.join("assets/gui", n + ".png"), "rb").read()).decode()

cards = ""
for i, (num, en, fa, desc, verdict, score, hat) in enumerate(OPTS, 1):
    hatched = ('<span class="hat">۲ برچسب تکراری</span>' if hat
               else '<span class="ok">۱۰ ماژول درست</span>')
    cards += f"""
<section class="card">
  <div class="head">
    <div class="n">{i:02d}</div>
    <div><h2>{en}</h2><div class="fa">{fa}</div></div>
    <div class="flagbox">{hatched}<span class="score">امتیاز {score}</span></div>
  </div>
  <img src="{b64(num)}" alt="{en}" loading="lazy">
  <p class="desc">{desc}</p>
  <p class="value">{verdict}</p>
</section>"""

HTML = f"""<!DOCTYPE html>
<html lang="fa" dir="rtl"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MH SIGNATURE — ۱۰ رابط کاربری، دستهٔ اول</title>
<style>
 :root {{ --bg:#07080a; --card:#0c0e12; --line:#1c2028; --ink:#eef1f5;
          --mut:#8b93a0; --red:#d42f38; --red2:#8f1b22; }}
 * {{ box-sizing:border-box; }}
 body {{ margin:0; background:var(--bg); color:var(--ink); font-size:14.5px;
   line-height:1.85; font-family:"Vazirmatn","IRANSans","Segoe UI",Tahoma,sans-serif;
   background-image:radial-gradient(1000px 460px at 82% -6%, #1a0d11 0%, transparent 60%),
                    radial-gradient(800px 400px at 2% 105%, #1e0d10 0%, transparent 60%); }}
 .wrap {{ max-width:1240px; margin:0 auto; padding:44px 18px 96px; }}
 header {{ text-align:center; border-bottom:1px solid var(--line); padding-bottom:26px; }}
 .logo {{ font-size:38px; letter-spacing:.44em; text-indent:.44em; font-weight:200;
   margin:0; text-transform:uppercase; }}
 .by {{ color:var(--mut); font-size:12px; letter-spacing:.26em; text-transform:uppercase; }}
 .lead {{ color:#b7bfca; max-width:800px; margin:18px auto 0; font-size:14.5px; }}
 .tags {{ display:flex; gap:7px; flex-wrap:wrap; justify-content:center; margin-top:20px; }}
 .tag {{ font-family:ui-monospace,monospace; font-size:11.5px; letter-spacing:.13em;
   border:1px solid var(--line); border-radius:20px; padding:4px 12px; color:#b6bec9;
   background:#0f1216; }}
 .card {{ background:var(--card); border:1px solid var(--line); border-radius:16px;
   padding:18px; margin:24px 0; }}
 .card img {{ width:100%; display:block; border-radius:10px; border:1px solid #15191f; }}
 .head {{ display:flex; align-items:center; gap:14px; margin-bottom:14px; flex-wrap:wrap; }}
 .n {{ font-size:23px; font-weight:200; color:var(--red); font-family:ui-monospace,monospace;
   border:1px solid var(--red2); border-radius:10px; padding:1px 11px; }}
 h2 {{ margin:0; font-size:17.5px; letter-spacing:.18em; font-weight:500;
   text-transform:uppercase; }}
 .fa {{ color:var(--mut); font-size:13px; }}
 .flagbox {{ margin-right:auto; display:flex; gap:7px; align-items:center; }}
 .hat {{ font-size:11.5px; border-radius:20px; padding:3px 10px;
   background:#1e0f11; border:1px solid #54262a; color:#e0898d; }}
 .ok {{ font-size:11.5px; border-radius:20px; padding:3px 10px;
   background:#0f1614; border:1px solid #22423a; color:#77bda8; }}
 .score {{ font-size:11.5px; border-radius:20px; padding:3px 10px;
   background:#12161c; border:1px solid #2b333d; color:#c2ccd8;
   font-family:ui-monospace,monospace; }}
 .desc {{ color:#c3cbd6; font-size:14px; margin:13px 0 6px; }}
 .value {{ color:#9aa3ae; font-size:13px; border-right:2px solid var(--red);
   padding-right:11px; margin:6px 0 2px; }}
 .next {{ margin:32px 0 0; padding:20px 22px; border:1px solid var(--red2);
   border-radius:14px; background:linear-gradient(180deg,#140b0d,#0a0b0e); }}
 .next h3 {{ margin:0 0 8px; color:var(--red); font-size:15px; letter-spacing:.06em; }}
 .next p {{ margin:6px 0; font-size:14px; color:#c9d0da; }}
 footer {{ margin-top:36px; padding-top:16px; border-top:1px solid var(--line);
   color:var(--mut); font-size:12.3px; text-align:center; }}
 code {{ direction:ltr; display:inline-block; background:#181c22; padding:1px 6px;
   border-radius:5px; font-size:12px; }}
</style></head><body><div class="wrap">

<header>
  <h1 class="logo">MH Signature</h1>
  <div class="by">by Milad Hashemi · GUI batch 1 / 2</div>
  <p class="lead">ده رابط کاربری روی معماری نهایی: <b>Signature Engine + ۹ ماژول رَک</b>
  با تب‌های ALL · EQ · DYN · COLOR · SPACE · MOD · IMAGE، و چهرهٔ میلاد هاشمی
  به‌عنوان بخشی از خودِ طراحی.</p>
  <div class="tags">
    <span class="tag">SIGNATURE ENGINE</span><span class="tag">EQ</span>
    <span class="tag">DYNAMICS</span><span class="tag">VOCAL</span>
    <span class="tag">COLOR</span><span class="tag">SPACE</span>
    <span class="tag">DELAY</span><span class="tag">IMAGE</span>
    <span class="tag">MODULATION</span><span class="tag">TRANSIENT</span>
    <span class="tag">UTILITY</span>
  </div>
</header>

{cards}

<div class="next">
  <h3>دستهٔ دوم — ۱۰ طرح دیگر</h3>
  <p>سقف تولید تصویر هر نوبت ۱۰ عدد است. بگویید «ادامه بده» تا ده طرح بعدی را بسازم:
  طرح‌های آنالوگ-صنعتی، سوئیسی، سینمایی ۲۱:۹، هافتُن، بلوپرینت، شیشهٔ دودی،
  رَک عمودی ۵۰۰-سری، و چند ایدهٔ کاملاً غیرمتعارف دیگر.</p>
</div>

<footer>
  MH SIGNATURE · دستهٔ ۱ از ۲ · پرتره: <code>assets/brand/milad-portrait-noir.png</code><br>
  تصاویر: <code>assets/gui/v5-*.png</code> · بدون طلایی، بدون زرد
</footer>
</div></body></html>"""

open("assets/gui/gallery-v5a.html", "w").write(HTML)
print("ok", os.path.getsize("assets/gui/gallery-v5a.html") // 1024, "KB")
