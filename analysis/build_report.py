#!/usr/bin/env python3
"""Build a self-contained Persian HTML report from the analysis JSONs."""
import json, base64, os, glob
import numpy as np

S = json.load(open("analysis/signature.json"))
S2 = json.load(open("analysis/signature2.json"))
st, rows = S["stats"], S["per_track"]
FIG = "analysis/figs"


def img(name, cap=""):
    p = os.path.join(FIG, name)
    if not os.path.exists(p):
        return ""
    b = base64.b64encode(open(p, "rb").read()).decode()
    return (f'<figure><img src="data:image/png;base64,{b}" alt="{name}">'
            f'<figcaption>{cap}</figcaption></figure>')


def med(k, d=1):
    v = st.get(k)
    return f"{v['median']:.{d}f}" if v else "—"


def rng(k, d=1):
    v = st.get(k)
    return f"{v['lo']:.{d}f} تا {v['hi']:.{d}f}" if v else "—"


oct_curve = S2["octave_curve_db"]
peaks = S2["recurring_peaks"]
closest, outliers = S2["closest"], S2["outliers"]

# ---------------------------------------------------------------- table
tbl = []
for r in sorted(rows, key=lambda x: -(x["lufs"] or -99)):
    tbl.append(
        "<tr>"
        f"<td class='name'>{r['name']}</td>"
        f"<td>{r['lufs']:.1f}</td><td>{r['lra']:.1f}</td><td>{r['tp']:+.1f}</td>"
        f"<td>{r['crest']:.1f}</td><td>{r['ms']:.1f}</td><td>{r['corr']:.2f}</td>"
        f"<td>{r['tilt']:.1f}</td><td>{r['perc_pct']:.0f}</td>"
        f"<td>{r['duck']:+.1f}</td><td>{r['t60_bb']:.1f}</td>"
        f"<td>{r['bpm']:.0f}</td><td>{r['key']}</td>"
        "</tr>")

bands = [("sub", "Sub 20–45"), ("bass", "Bass 45–90"), ("low_bass", "LowBass 90–140"),
         ("low_mid", "LowMid 140–300"), ("mid", "Mid 300–1k"), ("up_mid", "UpMid 1–2.5k"),
         ("presence", "Presence 2.5–5k"), ("brill", "Brill 5–9k"),
         ("air", "Air 9–14k"), ("top", "Top 14–20k")]
brows = "".join(f"<tr><td>{n}</td><td>{med(k)}</td><td>{rng(k)}</td></tr>" for k, n in bands)

wbands = [("sub", "Sub"), ("bass", "Bass"), ("low_bass", "LowBass"), ("low_mid", "LowMid"),
          ("mid", "Mid"), ("up_mid", "UpMid"), ("presence", "Presence"),
          ("brill", "Brill"), ("air", "Air")]
wrows = "".join(f"<tr><td>{n}</td><td>{med('w_'+k)}</td><td>{med('crest_'+k) if ('crest_'+k) in st else '—'}</td></tr>"
                for k, n in wbands)

octrows = "".join(f"<tr><td>{k} Hz</td><td>{v:+.1f}</td></tr>" for k, v in oct_curve.items())
peakrows = "".join(f"<tr><td>≈{int(hz)} Hz</td><td>{n} از ۲۲</td></tr>" for hz, n in peaks)

HTML = f"""<!DOCTYPE html>
<html lang="fa" dir="rtl">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>امضای میکس میلاد هاشمی — تحلیل ۲۲ اثر</title>
<style>
  :root {{ --bg:#0f1216; --card:#171c22; --ink:#e8edf3; --mut:#93a1b0;
           --acc:#d4a04a; --hot:#c0453f; --cool:#4d84c4; --line:#252c34; }}
  * {{ box-sizing:border-box; }}
  body {{ margin:0; background:var(--bg); color:var(--ink);
         font-family:"Vazirmatn","IRANSans","Segoe UI",Tahoma,sans-serif;
         line-height:1.9; font-size:15px; }}
  .wrap {{ max-width:1080px; margin:0 auto; padding:32px 20px 80px; }}
  header {{ border-bottom:1px solid var(--line); padding-bottom:22px; margin-bottom:30px; }}
  h1 {{ font-size:27px; margin:0 0 8px; font-weight:700; letter-spacing:-.2px; }}
  .sub {{ color:var(--mut); font-size:14px; }}
  h2 {{ font-size:20px; margin:44px 0 14px; padding-right:12px;
        border-right:4px solid var(--acc); }}
  h3 {{ font-size:16px; margin:26px 0 8px; color:var(--acc); }}
  p {{ margin:9px 0; }}
  .card {{ background:var(--card); border:1px solid var(--line); border-radius:12px;
           padding:16px 18px; margin:14px 0; }}
  .grid2 {{ display:grid; grid-template-columns:repeat(auto-fit,minmax(240px,1fr)); gap:12px; }}
  .kpi {{ background:var(--card); border:1px solid var(--line); border-radius:12px; padding:14px; }}
  .kpi .v {{ font-size:25px; font-weight:700; color:var(--acc); direction:ltr; text-align:right; }}
  .kpi .l {{ font-size:12.5px; color:var(--mut); }}
  table {{ width:100%; border-collapse:collapse; margin:12px 0; font-size:13px; }}
  th, td {{ padding:7px 9px; border-bottom:1px solid var(--line); text-align:center;
            font-variant-numeric:tabular-nums; direction:ltr; }}
  th {{ color:var(--mut); font-weight:600; font-size:12px; direction:rtl; }}
  td.name {{ text-align:left; direction:ltr; font-size:12px; color:#cfd8e2; }}
  tbody tr:hover {{ background:#1c232b; }}
  figure {{ margin:18px 0; background:var(--card); border:1px solid var(--line);
            border-radius:12px; padding:10px; }}
  figure img {{ width:100%; display:block; border-radius:6px; }}
  figcaption {{ color:var(--mut); font-size:12.5px; padding:8px 6px 2px; text-align:center; }}
  .tag {{ display:inline-block; padding:2px 9px; border-radius:20px; font-size:12px;
          background:#242c35; color:#c6d2de; margin:2px 3px; }}
  .hot {{ color:var(--hot); }} .cool {{ color:var(--cool); }} .acc {{ color:var(--acc); }}
  ul {{ padding-right:20px; }} li {{ margin:5px 0; }}
  .note {{ border-right:3px solid var(--hot); background:#1d1618; padding:12px 14px;
           border-radius:8px; font-size:13.5px; }}
  code {{ direction:ltr; display:inline-block; background:#242c35; padding:1px 6px;
          border-radius:5px; font-size:12.5px; }}
  .sig {{ font-size:14px; }}
  footer {{ margin-top:50px; padding-top:18px; border-top:1px solid var(--line);
            color:var(--mut); font-size:12.5px; }}
</style>
</head>
<body><div class="wrap">

<header>
  <h1>امضای میکس میلاد هاشمی</h1>
  <div class="sub">تحلیل اندازه‌گیری‌شدهٔ ۲۲ میکس نهایی · MP3 320kbps · منبع: پوشهٔ <code>Milad</code> در گوگل درایو</div>
</header>

<h2>۱. خلاصهٔ مدیریتی — امضای صوتی در یک نگاه</h2>
<div class="grid2">
  <div class="kpi"><div class="v">{med('lufs')} LUFS</div><div class="l">بلندی یکپارچه (میانه) — رنج {rng('lufs')}</div></div>
  <div class="kpi"><div class="v">{med('plr')} dB</div><div class="l">Peak-to-Loudness — فشرده‌سازی شدید</div></div>
  <div class="kpi"><div class="v">{med('tilt')} dB/oct</div><div class="l">شیب طیفی ۱۵۰Hz–۸kHz</div></div>
  <div class="kpi"><div class="v">{med('centroid',0)} Hz</div><div class="l">مرکز ثقل طیفی — تیره و گرم</div></div>
  <div class="kpi"><div class="v">{med('ms')} dB</div><div class="l">Side/Mid کل — تصویر استریوی محافظه‌کار</div></div>
  <div class="kpi"><div class="v">{med('corr')}</div><div class="l">همبستگی L/R — سازگار با مونو</div></div>
</div>

<div class="card sig">
<b>در یک جمله:</b> میکس‌های او <b>تیره، گرم، باس‌محور و به‌شدت بلند</b> هستند؛ بدنهٔ میان‌فرکانسی پر و روغنی،
ناحیهٔ Presence (۲.۵–۵kHz) عمداً خالی و فروکاسته، بالای ۵kHz ملایم و بدون «درخشش»، باس و ساب‌باس
کاملاً مونو و محکم، و بالای ۱۴۰Hz تصویر استریو به‌تدریج باز می‌شود. رِوِرب بلندتر از حد معمول با
میراییِ فرکانس‌بالا، و لیمیت/کلیپ هارد تا آستانهٔ دیجیتال.
</div>

<h2>۲. رنگ و جنس میکس (Tonal Colour &amp; Texture)</h2>

<h3>۲.۱ منحنی امضا در باندهای اکتاو</h3>
<p>میانگین طیف بلندمدت هر ۲۲ اثر، نرمال‌شده نسبت به باند ۶۴۰Hz–۱.۲kHz. این همان «رنگ» ثابتی است
که در تمام آثار تکرار می‌شود:</p>
<table>
<thead><tr><th>باند اکتاو</th><th>انحراف از مرکز میانی (dB)</th></tr></thead>
<tbody>{octrows}</tbody></table>
{img("signature_curve.png", "منحنی امضای تُنال — میانهٔ ۲۲ اثر. قرمز: تقویت‌شده، آبی: فروکاسته.")}

<h3>۲.۲ بالانس باندها (سهم انرژی هر باند از کل)</h3>
<table>
<thead><tr><th>باند</th><th>میانه (dB)</th><th>رنج در ۲۲ اثر</th></tr></thead>
<tbody>{brows}</tbody></table>
{img("band_heatmap.png", "نقشهٔ حرارتی بالانس باندها برای هر اثر — الگو در همه تکرار می‌شود: میانهٔ پر، Presence خالی.")}
{img("avg_spectrum.png", "میانگین طیف بلندمدت: خط قرمز میانهٔ ۲۲ اثر، ناحیهٔ رنگی فاصلهٔ چارک‌ها. افت تدریجی و بدون قلهٔ تیز.")}

<h3>۲.۳ چه چیزی این رنگ را می‌سازد؟</h3>
<ul>
  <li><b>بم شناور و پرحجم:</b> باند ۴۰–۸۰Hz به‌اندازهٔ <span class="hot">+۱۵.۴dB</span> بالاتر از مرکز میانی
      و ۸۰–۳۲۰Hz هم <span class="hot">+۸ تا +۹dB</span>. جنس باس «چاق و روغنی» است، نه خشک و پانچی.</li>
  <li><b>میانهٔ ۳۰۰Hz–۱kHz نقطهٔ اتکای میکس:</b> پرانرژی‌ترین ناحیهٔ بالای باس. وکال و سازهای
      اصلی دقیقاً همین‌جا می‌نشینند.</li>
  <li><b>غارِ Presence:</b> ۲.۵–۵kHz حدود <span class="cool">−۱۲.۸dB</span> و ۵–۱۰kHz حدود
      <span class="cool">−۱۶.۵dB</span> پایین‌تر از میانه. این «سوراخ» عمدی است و همان چیزی است که
      صدا را <b>نرم، غیرخشن و آنالوگ‌گونه</b> می‌کند — به قیمت کمبود «هوای» بالا و وضوح.</li>
  <li><b>بالای ۱۰kHz کاملاً خاموش:</b> حدود <span class="cool">−۲۸dB</span>. هیچ هایپ یا شیمرِ روشنی در کار نیست.</li>
  <li><b>شیب طیفی {med('tilt')} dB/oct:</b> تندتر از ۴.۵dB/oct متعارف؛ یعنی با هر اکتاو رفتن به بالا،
      انرژی سریع‌تر از حالت طبیعی می‌افتد → حس «گرم و شبیه نوار».</li>
  <li><b>مرکز ثقل {med('centroid',0)}Hz و Rolloff95 ≈ {med('rolloff95',0)}Hz:</b> ۹۵٪ انرژی زیر ۲.۵kHz است. یعنی
      میکس عملاً «میان‌محور» است، نه بالا-محور.</li>
</ul>

<h2>۳. تصویر استریو</h2>
<table>
<thead><tr><th>باند</th><th>Side/Mid (dB)</th><th>Crest Factor (dB)</th></tr></thead>
<tbody>{wrows}</tbody></table>
{img("width_by_band.png", "پهنای استریو به تفکیک باند — میانه (خط نارنجی) و دامنهٔ تغییرات در ۲۲ اثر.")}
<p>یک الگوی کاملاً کلاسیک و آگاهانه دیده می‌شود:</p>
<ul>
  <li><b>Sub {med('w_sub')}dB و Bass {med('w_bass')}dB:</b> عملاً هیچ محتوای Side‌ای زیر ۹۰Hz نیست — باس
      و کیک کاملاً مونو. نتیجه: پایین‌دست محکم و مونو-سازگار.</li>
  <li>باز شدن تدریجی از ۱۴۰Hz به بالا: LowMid {med('w_mid')}dB → Mid → Presence {med('w_pres')}dB →
      Air <span class="acc">{med('w_air')}dB</span> (بازترین لایه).</li>
  <li>همبستگی L/R بین {rng('corr',2)}؛ یعنی هرگز به‌سمت فاز-مشکل یا وایدِ افراطی نمی‌رود.
      استریو ابزار «فضاسازی» است، نه نمایش.</li>
</ul>

<h2>۴. بلندی، داینامیک و مسترینگ</h2>
{img("lufs.png", "بلندی یکپارچهٔ هر اثر در برابر مرجع ‎−۱۴ LUFS‎.")}
<div class="grid2">
  <div class="kpi"><div class="v">{med('lufs')} LUFS</div><div class="l">بلندی میانه · رنج {rng('lufs')}</div></div>
  <div class="kpi"><div class="v">{med('lra')} LU</div><div class="l">LRA · رنج {rng('lra')}</div></div>
  <div class="kpi"><div class="v">+{med('tp')} dBTP</div><div class="l">پیک واقعی (پس از دیکد MP3)</div></div>
  <div class="kpi"><div class="v">{med('crest')} dB</div><div class="l">Crest Factor · رنج {rng('crest')}</div></div>
</div>
{img("truepeak.png", "پیک واقعی هر اثر — همه بالای صفر دسی‌بل؛ نشانهٔ لیمیت/کلیپِ هارد.")}
<ul>
  <li><b>بلندی بسیار بالا:</b> میانهٔ {med('lufs')} LUFS یعنی حدود <b>۶.۴dB بلندتر از مرجع استریم</b>.
      رنج باریک ({rng('lufs')}) نشان می‌دهد این یک <i>سیاست ثابت مسترینگ</i> است، نه اتفاق.</li>
  <li><b>داینامیک محدود:</b> PLR میانه {med('plr')}dB و Crest {med('crest')}dB. LRA فقط {med('lra')} LU —
      یعنی بین آرام‌ترین و بلندترین بخش آهنگ تقریباً فشاری وجود ندارد.</li>
  <li><b>کلیپ واقعی روی مستر:</b> به‌طور میانه <b>{med('clip_pct',2)}٪</b> نمونه‌ها روی سقف دیجیتال
      (‎−۰.۳dBFS‎) می‌نشینند و تا <b>{rng('flat_pct',2)}٪</b> در ران‌های flat-top قرار دارند.
      این «چسبندگی» و گرما بخشی از جنس صدا است — و بخشی از دلیل TP مثبت.</li>
  <li><b>بدون لوپس/فیلتر بالا:</b> محتوا تا نزدیکی ۲۰kHz ادامه دارد (سقف باند MP3)؛ فایل‌ها فول‌باند هستند.</li>
</ul>

<h2>۵. پردازش و افکت لاین‌ها — آنچه از میکس قابل استنتاج است</h2>
<div class="note">
<b>محدودیت مهم:</b> ورودی من فقط <b>میکس نهایی مسترشده</b> است، نه استم‌ها و نه پروژهٔ DAW.
بنابراین بخش‌های زیر <b>استنتاج آماری</b> از نشانه‌های آکوستیکی است، نه خواندنِ زنجیرهٔ افکت.
هر جا اطمینان کمتر است، صریح گفته‌ام.
</div>

<h3>۵.۱ وکال</h3>
<ul>
  <li><b>جایگاه:</b> انرژی وکال در باند میانی ۳۰۰Hz–۱kHz و بدنهٔ ۱۴۰–۳۰۰Hz می‌نشیند؛
      با فروکاستن Presence، وکال «جلو» می‌آید ولی <b>تیز نمی‌شود</b> — تکنیک رایج «وکال گرم/آنالوگ».</li>
  <li><b>دِ-اِسینگ و کنترل سیبیلانس:</b> ناحیهٔ ۶–۹kHz حدود {med('sib')}dB پایین‌تر از ۳–۵kHz است.
      یعنی یا های‌شلف روی وکال، یا دِاِس قوی، یا هر دو — سیبیلانس عملاً حذف شده.</li>
  <li><b>فشرده‌سازی:</b> Crest باند میانی {med('crest_mid')}dB در مقابل Presence {med('crest_pres')}dB و
      Air {med('crest_air')}dB. یعنی بدنهٔ وکال <b>به‌شدت فشرده و چسبنده</b> است، در حالی که بالاتر
      دست‌نخورده‌تر مانده → «وکال ضخیم و ثابت، با کمی درخشش طبیعی در لحظه‌های خاص».</li>
  <li><b>فضا/افکت:</b> رِوِرب و دِلی با میرایی فرکانس‌بالا؛ T60 برآوردی —
      میانه {med('t60_mid')} ثانیه در مید، {med('t60_high')} ثانیه در بالا، {med('t60_low')} ثانیه در بم.
      <b>مید بلندتر از بم</b> یعنی ریورب روی وکال/سازها پهن است ولی لُو-اند با فیلتر بالا کنترل شده
      (های‌پس روی سِند ریورب) — تا باس گل‌آلود نشود. الگوی «هال/پلیت با دکِی، فیلترشده در پایین».</li>
  <li><b>دابل/هارمونی:</b> پهنای Side در Mid نسبتاً زیاد است ({med('w_mid')}dB) که با دابلینگ یا
      استریو-دِلِی کوتاه روی وکال سازگار است.</li>
</ul>

<h3>۵.۲ درام و باس</h3>
<ul>
  <li><b>باس/کیک:</b> Coral تنال ۴۰–۸۰Hz و مونو بودن کامل زیر ۹۰Hz، همراه با T60 کوتاهِ بم
      ({med('t60_low')}s) نشان‌دهندهٔ <b>کنترل جدی روی لُو-اند</b>: مانیتورینگ دقیق، های‌پس روی
      همه‌چیز جز کیک/باس، و سایدچین.</li>
  <li><b>سایدچین / داکینگ:</b> میانهٔ <span class="acc">{med('duck')}dB</span> افت در ۵۰–۱۵۰ms بعد از هر کیک،
      با رنج {rng('duck')}dB. یعنی سایدچین <b>وجود دارد ولی در بیشتر آثار ملایم</b> است؛ در چند اثر
      (مثل ‎06_FORSAT‎ و ‎08_GHALBE MAN‎) به ۲–۳dB می‌رسد و «پامپ» محسوس می‌شود.
      مُدولاسیون پاکت با دورهٔ ~{med('mod_bpm',0)}BPM تأیید می‌کند که این داکینگ با تمپو قفل است.</li>
  <li><b>جنس کیک:</b> Crest باند بم {med('crest_bass')}dB — پانچ هست، ولی با کلیپ/لیمیت به بدنهٔ
      فشرده چسبانده شده.</li>
</ul>

<h3>۵.۳ سازها و لایهٔ هارمونیک</h3>
<ul>
  <li><b>نسبت هارمونیک به پرکاسیو:</b> {med('harm_pct',0)}٪ هارمونیک در برابر {med('perc_pct',0)}٪ پرکاسیو.
      میکس «آهنگ‌محور» است: ملودی و پد و سازها بر ریتم غلبه دارند. (رنج پرکاسیو {rng('perc_pct',0)}٪
      که در قطعه‌های تندتر بالا می‌رود.)</li>
  <li><b>پد/کیبورد/گیتار:</b> با توجه به باز شدن پهنای استریو بالای ۱۴۰Hz و Side بالا در Air،
      لایه‌های هارمونیک <b>پهن و دیفیوز</b> پن می‌شوند (احتمالاً استریو-دِلِی/کورس و پن کردن دو-طرفه)
      ولی همیشه زیر وکال می‌مانند.</li>
  <li><b>افکت‌های فضایی:</b> میراییِ بالای ریورب + T60 میدِ بلند، الگوی «فضای بزرگ اما تاریک» است
      (هال یا پلیت با LPF روی سِند) — نه فضای روشن و درخشان.</li>
  <li><b>سچوریشن:</b> کلیپ هاردِ اندازه‌گیری‌شده روی مستر ({med('flat_pct',2)}٪ flat-top) و شیب تند
      بالای ۵kHz با همان الگوی «هِیرمونیک اضافه شده + هارمونیک‌های بالا کمتر از حد» سازگار است.
      احتمال بالای استفاده از سچوریشن/تیپ/کنسول-اِمولیشن روی باس یا مستر (این مورد <b>احتمالی</b> است).</li>
</ul>

<h2>۶. الگوهای تکرارشونده و استثناها</h2>
<div class="grid2">
  <div class="card"><b>قله‌های طیفی تکرارشونده</b>
    <table><thead><tr><th>فرکانس</th><th>تعداد آثار</th></tr></thead><tbody>{peakrows}</tbody></table>
    <p style="font-size:13px;color:#93a1b0">قلهٔ ~۱۵۰Hz در ۷ اثر: احتمالاً فرکانس بنیادِ باس/کیک یا
    یک شلف ثابت روی باس. تنها قلهٔ مشترک قابل‌اتکا — بقیه طیف صاف و بدون رزونانس آزاردهنده است
    (نشانهٔ کنترل و مانیتورینگ خوب).</p>
  </div>
  <div class="card"><b>انسجام امضا</b>
    <p style="font-size:13px">پراکندگی درونی بسیار کم است: انحراف معیار بلندی ۰.۷dB، شیب طیفی ۰.۵dB/oct،
    باند میانی ۰.۷dB. یعنی یک «صدای خانه» ثابت، مستقل از خواننده و سبک.</p>
    <p style="font-size:13px">نزدیک‌ترین آثار به مرکز امضا: {", ".join(n.split(' - ')[-1] for n in closest[:4])}</p>
    <p style="font-size:13px">پرتفاوت‌ترین‌ها: {", ".join(n.split(' - ')[-1] for n in outliers)}</p>
  </div>
</div>
{img("similarity.png", "فاصلهٔ بالانس تُنال بین آثار. اکثر آثار در یک خوشهٔ فشرده‌اند؛ چند اثر (ETERAF، Tamasha، FORSAT، TASVIR) کمی متفاوت‌اند.")}

<h2>۷. جدول کامل اندازه‌گیری‌ها</h2>
<table>
<thead><tr>
  <th>اثر</th><th>LUFS</th><th>LRA</th><th>TP</th><th>Crest</th><th>M/S</th>
  <th>Corr</th><th>Tilt</th><th>Perc%</th><th>Duck</th><th>T60</th><th>BPM</th><th>Key</th>
</tr></thead>
<tbody>{''.join(tbl)}</tbody></table>
<p style="font-size:12.5px;color:#93a1b0">TP = پیک واقعی پس از دیکد MP3 (بخشی از آن overshoot کدک است،
اما مثبت بودن آن در همهٔ ۲۲ اثر یعنی مستر از قبل کلیپ/لیمیت شده). Duck = افت dB در ۵۰–۱۵۰ms بعد از کیک
(منفی = داکینگ). T60 = برآورد زمان افت ۶۰dB از پاکت باندپس — تقریبی.</p>

<h2>۸. آنچه هنوز نمی‌دانیم (و برای فهمیدنش چه لازم است)</h2>
<ul>
  <li><b>نوع و تنظیمات دقیق پلاگین‌ها:</b> از میکس نهایی قابل استخراج نیست. برای آن به استم‌های جدا یا پروژهٔ DAW نیاز است.</li>
  <li><b>زنجیرهٔ وکال (ترتیب EQ/کمپرسور/سچوریشن، پارامترها):</b> فقط الگوی کلی («بدنهٔ فشرده + Presence خالی + سیبیلانس مهارشده») استنباط می‌شود.</li>
  <li><b>تفکیک ریورب از دِلِی و از خودِ ساز:</b> T60 برآوردی مجموع فضای میکس است.</li>
  <li><b>میزان دقیق کلیپینگ در مستر قبل از MP3:</b> کدک MP3 روی پیک اثر می‌گذارد.</li>
</ul>

<footer>
  تولید‌شده با موتور آنالیز اختصاصی MH-SIGNATURE · ۲۲ فایل، مجموعاً ~۱۹۵MB ورودی ·
  معیارها: EBU R128 (ffmpeg), Welch PSD 8k, Mid/Side per-band, HPSS, envelope-decay, kick-duck detection.
  داده‌های خام: <code>analysis/summary.csv</code> و <code>analysis/out/*.json</code>.
</footer>
</div></body></html>
"""

open("analysis/report.html", "w").write(HTML)
print("wrote analysis/report.html", os.path.getsize("analysis/report.html") // 1024, "KB")
