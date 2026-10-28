#!/usr/bin/env python3
"""MH SIGNATURE — v6 gallery, batch 1 (10 designs, every knob labelled)."""
import base64, os

GLASS = [
    ("v6-01-glass-grid", "GLASS GRID", "شیشهٔ مشکی · شبکهٔ منظم",
     "پنل‌های شیشهٔ مشکی پولیش با هایلایت تیز و رگه‌های قرمز که از زیر شیشه عبور می‌کنند. "
     "پرتره در پنل شیشه‌ای چپ، موتور امضا در مرکز، آنالایزر در راست و ۱۰ کارت ماژول پایین.",
     "همهٔ ناب‌ها برچسب دارند: ENGINE · CHARACTER · DENSITY · SPACE · WIDTH و در کارت‌ها LOW/MUD/BODY/AIR · THR/RATIO/ATK/REL · PRES/GAIN · DRIVE/TONE/MIX/GAIN · MIX · SIZE/DAMP/MIX/GAIN · WIDTH/GAIN · RATE/DEPTH/MIX/GAIN · ATK/REL/GAIN · INPUT/GAIN"),
    ("v6-02-glass-lens", "GLASS LENS", "شیشهٔ مشکی · لنز دایره‌ای",
     "پرتره در یک لنز شیشهٔ مشکی با رینگ قرمز نازک. کارت‌ها شیشه‌ای با گوشه‌های گرد و "
     "انعکاس عمیق؛ رگه‌های قرمز به‌وضوح از زیر سطح شیشه می‌گذرند.",
     "برچسب‌ها: ENGINE · CHARACTER · DENSITY · SPACE · WIDTH · LOW/MID · HIGH · THR/RATIO/ATK/REL · MUD/BODY/PRES/AIR · TONE/DRIVE · SIZE/MIX · RATE/WIDTH · SIZE/DAMP/MIX · WIDTH/GAIN · RATE/DEPTH · ATK/REL"),
    ("v6-03-glass-tiles", "GLASS TILES", "شیشهٔ مشکی · کاشی‌های براق",
     "شبکهٔ دقیق کاشی‌های شیشه‌ای ضخیم با رفلکس‌های تیز و رگه‌های قرمز درون هر کاشی. "
     "پرتره در کاشی بلند سمت راست.",
     "برچسب‌ها: ENGINE · CHARACTER/DENSITY/SPACE/WIDTH · LOW/MUD/BODY/PRES/AIR · THR/RATIO/ATK/REL/GAIN · DE-ESS/DRIVE/PRESENCE/MIX · DRIVE/CHARACTER/TONE/OUTPUT · MIX/SIZE/DAMP/PRE-DELAY/DECAY · MIX/FEEDBACK/TIME/RATE · WIDTH/PAN/PHASE/CROSS · RATE/DEPTH/OFFSET/SYNC · ATTACK/SUSTAIN/PUNCH/DRIVE"),
    ("v6-04-glass-rows", "GLASS ROWS", "شیشهٔ مشکی · ردیف‌های افقی",
     "یک سطح شیشهٔ مشکی یکپارچه با هایلایت مورب، و ده ردیف افقی — هر ردیف یک ماژول با دو ناب برچسب‌دار.",
     "برچسب‌ها: ENGINE · CHARACTER/DENSITY/SPACE/WIDTH · FREQ/GAIN · THR/MIX · THR/MIX · FREQ/GAIN · SIZE/DAMP · SIZE/DAMP · SIZE/DAMP · MOD/MIX · SIZE/DAMP · ONGP/GAINT"),
    ("v6-05-glass-diagonal", "GLASS DIAGONAL", "شیشهٔ مشکی · رگه‌های مورب",
     "رگه‌های نئونی مورب روی شیشهٔ مشکی براق، مثل بازتاب نور در حرکت. "
     "پرتره در کاشی چپ‌پایین با امضای دستی و کلمات MUSIC/PEOPLE/EMOTION/ALWAYS.",
     "برچسب‌ها: ENGINE · CHARACTER/DENSITY/SPACE/WIDTH · LOW/MID/HIGH/MUD/BODY/PRES/AIR · THR/RATIO/GAIN · BODY/AIR · DRIVE/GAIN · MIX/DAMP · SIZE/RATE/DEPTH · RATE/DEPTH · THR/MIX"),
]

MATTE = [
    ("v6-06-matte-filaments", "MATTE FILAMENTS", "مات نوآر · رشته‌های نور",
     "پنل‌های آلومینیوم مات ذغالی با رشته‌های نازک قرمز که مثل فیبر نوری از پشت پنل‌ها می‌گذرند. "
     "ریل پرترهٔ چپ با تیتر عمودی، و ۱۰ کارت در شبکهٔ ۵×۲.",
     "برچسب‌ها: ENGINE · CHARACTER/DENSITY/SPACE/WIDTH · LOW/MUD · THR/GAIN · PRES/TONE · DRIVE/TRIM · MIX/FEEDBACK · SIZE/DAMP · WIDTH/POS · RATE/DEPTH · ATK/REL · TRIM/GAIN"),
    ("v6-07-vein-marble", "VEIN MARBLE", "سنگ سیاه · رگه‌های قرمز",
     "سطحی مثل سنگ سیاه با رگه‌های معدنی قرمز که می‌درخشند؛ چهره در خودِ سنگ حک شده. "
     "تب‌ها، موتور امضا، آنالایزر و ۹ کارت ماژول.",
     "برچسب‌ها: ENGINE · CHARACTER/DENSITY/SPACE/WIDTH · LOW/MID/HIGH/BODY/PRES/AIR · THR/RATIO/GAIN/MIX · PRES/BODY/GAIN · DRIVE/GAIN · SIZE/DAMP · RATE/DEPTH/MIX · MIX/GAIN · RATE/DEPTH/MIX · DRIVE/MIX"),
    ("v6-08-carbon-laser", "CARBON LASER", "کربن سیاه · لیزر قرمز",
     "بافت تیرهٔ کربن با پرتره در یک روزنهٔ دایره‌ای شیشهٔ مشکی، و رگه‌های لیزری قرمز که سطح را می‌شکافند. "
     "۹ کارت ماژول در یک ردیف.",
     "برچسب‌ها: ENGINE · CHARACTER/DENSITY/SPACE/WIDTH · LOW/MUD/BODY/PRES · THR/RATIO · AIR · MIX · SIZE · DAMP · RATE/DEPTH · DRIVE"),
    ("v6-09-light-trails", "LIGHT TRAILS", "نوردهی طولانی · رد نور",
     "رگه‌های قرمز مثل عکاسی نوردهی طولانی دور چهره و کنترل‌ها می‌پیچند؛ "
     "پرتره در یک‌سوم راست، کنسول در چپ با ۸ کارت ماژول.",
     "برچسب‌ها: ENGINE · CHARACTER/DENSITY/SPACE/WIDTH · LOW/MID/HIGH · THR/RATIO · MUD/BODY/PRES/AIR · DRIVE/MIX · SIZE/DAMP · MIX · RATE/DEPTH"),
    ("v6-10-streak-spotlight", "STREAK SPOTLIGHT", "نورافکن قرمز · رگهٔ افقی",
     "یک نوار نور قرمز افقی مثل نورافکن روی صحنهٔ تاریک، که پشت ناب اصلی روشن‌ترین است. "
     "پرترهٔ نیمه‌روشن در چپ و ۱۰ نوار ماژول در دو ردیف.",
     "برچسب‌ها: ENGINE · CHARACTER/DENSITY/SPACE/WIDTH · LOW/HIGH · MUD/MID · PRES/ATTACK · THR/RATIO · MIX/PAN · SIZE/TIME · DAMP/FEEDBACK · RATE/DEPTH · DRIVE/GAIN"),
]

def b64(n):
    return "data:image/png;base64," + base64.b64encode(
        open(os.path.join("assets/gui", n + ".png"), "rb").read()).decode()

def card(i, item, glass):
    num, en, fa, desc, labels = item
    badge = ('<span class="glass">شیشهٔ مشکی براق</span>' if glass
             else '<span class="matte">مات نوآر</span>')
    return f"""
<section class="card">
  <div class="head">
    <div class="n">{i:02d}</div>
    <div><h2>{en}</h2><div class="fa">{fa}</div></div>
    <div class="flagbox">{badge}<span class="ok">همهٔ ناب‌ها برچسب دارند</span></div>
  </div>
  <img src="{b64(num)}" alt="{en}" loading="lazy">
  <p class="desc">{desc}</p>
  <p class="labels"><b>برچسب ناب‌ها:</b> {labels}</p>
</section>"""

sec1 = "".join(card(i, it, True) for i, it in enumerate(GLASS, 1))
sec2 = "".join(card(i, it, False) for i, it in enumerate(MATTE, 6))

HTML = f"""<!DOCTYPE html>
<html lang="fa" dir="rtl"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MH SIGNATURE — دستهٔ ۱: ۱۰ طرح با ناب‌های برچسب‌دار</title>
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
 .lead {{ color:#b7bfca; max-width:820px; margin:18px auto 0; font-size:14.5px; }}
 .band {{ margin:38px 0 10px; display:flex; align-items:center; gap:12px; }}
 .band h3 {{ margin:0; font-size:13px; letter-spacing:.22em; color:var(--red);
   text-transform:uppercase; white-space:nowrap; }}
 .band .rule {{ flex:1; height:1px; background:var(--line); }}
 .band .cnt {{ font-family:ui-monospace,monospace; font-size:11.5px; color:#7d8794; }}
 .card {{ background:var(--card); border:1px solid var(--line); border-radius:16px;
   padding:18px; margin:22px 0; }}
 .card img {{ width:100%; display:block; border-radius:10px; border:1px solid #15191f; }}
 .head {{ display:flex; align-items:center; gap:14px; margin-bottom:14px; flex-wrap:wrap; }}
 .n {{ font-size:23px; font-weight:200; color:var(--red); font-family:ui-monospace,monospace;
   border:1px solid var(--red2); border-radius:10px; padding:1px 11px; }}
 h2 {{ margin:0; font-size:17.5px; letter-spacing:.18em; font-weight:500;
   text-transform:uppercase; }}
 .fa {{ color:var(--mut); font-size:13px; }}
 .flagbox {{ margin-right:auto; display:flex; gap:7px; align-items:center; flex-wrap:wrap; }}
 .glass, .matte, .ok {{ font-size:11.5px; border-radius:20px; padding:3px 10px; }}
 .glass {{ background:#101820; border:1px solid #2b3a48; color:#9fc3dd; }}
 .matte {{ background:#141414; border:1px solid #333; color:#b8b8b8; }}
 .ok {{ background:#0f1614; border:1px solid #22423a; color:#77bda8; }}
 .desc {{ color:#c3cbd6; font-size:14px; margin:13px 0 6px; }}
 .labels {{ color:#93a0ae; font-size:12.4px; border-right:2px solid var(--red);
   padding-right:11px; margin:6px 0 2px; line-height:1.9; direction:ltr; text-align:left; }}
 .labels b {{ color:#c9d3de; direction:rtl; display:inline-block; }}
 .next {{ margin:30px 0 0; padding:20px 22px; border:1px solid var(--red2);
   border-radius:14px; background:linear-gradient(180deg,#140b0d,#0a0b0e); }}
 .next h3 {{ margin:0 0 8px; color:var(--red); font-size:15px; }}
 .next p {{ margin:6px 0; font-size:14px; color:#c9d0da; }}
 footer {{ margin-top:36px; padding-top:16px; border-top:1px solid var(--line);
   color:var(--mut); font-size:12.3px; text-align:center; }}
 code {{ direction:ltr; display:inline-block; background:#181c22; padding:1px 6px;
   border-radius:5px; font-size:12px; }}
</style></head><body><div class="wrap">

<header>
  <h1 class="logo">MH Signature</h1>
  <div class="by">by Milad Hashemi · batch 1 / 2 · every knob labelled</div>
  <p class="lead">در همهٔ ده طرح این دسته، <b>هر ناب یک برچسب دارد</b> — هیچ کنترل بی‌نامی نیست.
  تم: نوآر مشکی با رگه‌های قرمز، و چهرهٔ میلاد هاشمی درون طراحی.
  پنج طرح اول استایل <b>شیشهٔ مشکی براق</b> دارند.</p>
</header>

<div class="band"><h3>شیشهٔ مشکی براق · رگه‌های قرمز</h3>
  <div class="rule"></div><div class="cnt">۰۱ – ۰۵</div></div>
{sec1}

<div class="band"><h3>مات نوآر · رگه‌های قرمز</h3>
  <div class="rule"></div><div class="cnt">۰۶ – ۱۰</div></div>
{sec2}

<div class="next">
  <h3>دستهٔ دوم — ۱۰ طرح دیگر</h3>
  <p>سقف تولید تصویر هر نوبت ۱۰ عدد است. بگویید <b>«ادامه بده»</b> تا ده طرح بعدی را
  با همان قاعدهٔ برچسب‌دار بسازم: سوئیسی مینیمال، سینمایی ۲۱:۹، هافتُن چاپی،
  بلوپرینت فنی، رَک عمودی ۵۰۰-سری، شیشهٔ مات، مونوکروم قرمز، و چند ایدهٔ دیگر.</p>
</div>

<footer>
  MH SIGNATURE · دستهٔ ۱ از ۲ · تصاویر: <code>assets/gui/v6-*.png</code><br>
  بدون طلایی، بدون زرد · پرتره: <code>assets/brand/milad-portrait-noir.png</code>
</footer>
</div></body></html>"""

open("assets/gui/gallery-v6a.html", "w").write(HTML)
print("ok", os.path.getsize("assets/gui/gallery-v6a.html") // 1024, "KB")
