# MH SIGNATURE — by Milad Hashemi

**An all-in-one mixing environment.** Ten modules around one Signature Engine,
built from the measured signature of 22 mixes.

`VERSION 1.0.0 · AN ALL-IN-ONE MIXING ENVIRONMENT · DESIGNED BY MILAD HASHEMI`

---

## نصب افزونه (نسخه‌های ساخته‌شده در GitHub Actions)

هر بار که کد به‌روز می‌شود، GitHub Actions نسخه‌ها را می‌سازد. برای دانلود:

1. در GitHub به تب **Actions** بروید
2. آخرین اجرای موفق **Build MH SIGNATURE** را باز کنید
3. از بخش **Artifacts** یکی را دانلود کنید:
   - `MH-SIGNATURE-Windows-x64-VST3` — ویندوز ۶۴ بیتی
   - `MH-SIGNATURE-macOS-universal-AU-VST3` — مک (هم Intel هم Apple Silicon)
   - `MH-SIGNATURE-...-Standalone` — نسخهٔ مستقل برای تست سریع بدون DAW

### ویندوز (VST3)

فایل ZIP را باز کنید و پوشهٔ `MH SIGNATURE.vst3` را کپی کنید در:

```
C:\Program Files\Common Files\VST3\
```

سپس در DAW یک Rescan برای پوشهٔ VST3 بزنید. پلاگین در دستهٔ **Fx** ظاهر می‌شود.

### مک (AU + VST3)

```bash
unzip MH-SIGNATURE-macOS-universal-AU-VST3.zip

cp -R "MH SIGNATURE.component" ~/Library/Audio/Plug-Ins/Components/
cp -R "MH SIGNATURE.vst3"      ~/Library/Audio/Plug-Ins/VST3/
```

اگر مک از اجرای افزونه جلوگیری کرد:

```bash
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/"MH SIGNATURE.component"
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/"MH SIGNATURE.vst3"
```

هر دو نسخه **Universal** هستند: Intel و Apple Silicon.

---

## The instrument panel

| Module | Controls |
|---|---|
| **SIGNATURE ENGINE** | CHARACTER · DENSITY · SPACE · WIDTH · INTENSITY |
| **EQ** | HPF · 4 bands (65 / 152 / 3.5k / 9k) · TILT |
| **DYNAMICS · LIMITER** | threshold, ratio, attack, release, makeup, mix, ceiling, clip |
| **VOCAL CONTROL** | HPF, body, compressor, de-esser, presence, air, reverb send |
| **SATURATION · COLOR** | tape / console / tube / diode / hard-clip, drive, tone, HF roll-off, mix |
| **DELAY · REVERB** | ping-pong delay with damping · pre-delayed hall |
| **STEREO IMAGER** | mono-maker, four-band width, balance |
| **MODULATION** | chorus, rate / depth / mix |
| **TRANSIENT SHAPER** | attack, sustain, split frequency |
| **UTILITY · I/O** | input, output, mix, mono, bypass |

Header tabs `ALL | EQ | DYN | COLOR | SPACE | MOD | IMAGE` dim everything that is
not part of the selected family, so the eye lands on the right row.

**100 factory presets** in ten families: Signature, Ballad, Pop, Rock, Hip-Hop,
Electronic, Acoustic, Vocal Focus, Dark Room, Streaming.

The whole window is resizable — drag any corner. The layout scales as one unit,
so nothing ever overlaps, from 688 × 480 up to 2752 × 1920.

---

## Building from source

Requirements: CMake ≥ 3.22 and a C++17 toolchain. JUCE 7.0.12 is downloaded
automatically by CMake.

```bash
cmake -S plugin -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Artefacts land in `build/MHSignature_artefacts/Release/`.

Useful cache variables:

| Variable | Default | Meaning |
|---|---|---|
| `MH_FORMATS` | `VST3;AU;Standalone` | which formats to build |
| `CMAKE_OSX_ARCHITECTURES` | host | `x86_64;arm64` for a universal mac build |
| `CMAKE_OSX_DEPLOYMENT_TARGET` | — | `11.0` for the CI mac build |

---

## Repository map

```
plugin/                 the JUCE plugin
  Source/Parameters.h   75 automatable parameters + their defaults
  Source/DspChain.h     the ten-module signal path
  Source/Analyzer.h     the 96-band analyser behind the display
  Source/PluginEditor.* the MIRROR · D interface
  Source/Presets.*      100 factory presets (generated)
  tools/gen_presets.py  regenerates Presets.cpp
  assets/portrait.jpg   the embedded portrait
analysis/               the study of the 22 reference mixes
assets/gui/             the design boards
design/tokens.json      palette and spacing tokens
.github/workflows/      the three-platform build
```

Every default in `Parameters.h` is a measured number, not a guess: the 22 mixes
average −7.6 LUFS with a −5.37 dB/oct tilt, a 3.4 s low-mid tail, a −7.67 dB
side/mid ratio and a −12.8 dB presence dip. That is the sound this plugin starts
from before you touch a single knob.
