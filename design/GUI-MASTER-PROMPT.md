# MH SIGNATURE — GUI Master Prompt

Everything you need to generate interface concepts for **MH SIGNATURE — by Milad Hashemi**
in any image model (Midjourney, DALL·E, Flux, Nano Banana, Ideogram, SDXL…).

The prompt is deliberately *layout-agnostic*: it fixes the product, the palette, the
type scale and the rule that every module needs an ON/OFF switch, but leaves the
composition open so you can explore.

---

## How to use it

1. Copy **Prompt A** as-is. Add one line from **Prompt D** if you want a specific archetype
   (or nothing at all if you want the model to decide).
2. Generate at **16:10**. Ask for 1440 × 900 or larger; export at 2× (2880 × 1800).
3. When a result is close but not right, use the edit prompts in **Prompt E** — never
   re-roll from zero, refine the one you like.
4. Score the result with the checklist in **Prompt F** before sending it to me, so we
   don't spend a build cycle on something unbuildable.

> Text inside AI images is often imperfect. That is fine — I rebuild the interface in
> code, so what matters is the **composition, proportion and feel**, not the exact glyphs.

---

## Prompt A — the master prompt (copy this)

```
Design a complete user-interface mockup for a professional audio mixing plugin called
"MH SIGNATURE — by Milad Hashemi". Render it as a single, perfectly aligned interface
screenshot, seen straight on, no perspective, no device frame, no desk, no shadows
around the window. Canvas 1440 x 900 (16:10), export at 2x.

WHAT THE PRODUCT IS
A single-window mixing environment used inside a music DAW. Ten processing modules
arranged around one central "SIGNATURE ENGINE". The person who designed it is a mixing
engineer; the interface should feel like precision audio hardware interpreted by an
editorial designer: calm, dark, confident, nothing decorative for its own sake.

VISUAL LANGUAGE
Minimal, flat, precise. 1-pixel hairlines, generous negative space, one accent colour
used sparingly. Think: a high-end hardware rack unit drawn by a Swiss designer. No 3D
bevels, no chrome, no screws, no plastic gloss, no skeuomorphic wood, no glass
reflections, no neon, no HUD clichés, no futuristic holograms, no drop shadows except a
very soft radial glow behind the single hero control.

COLOUR — use exactly these
  background #050607
  panel #101216 with a 1px border #24272D
  accent #D0361C   (exactly this vermilion; used for active states, arcs, dots)
  deep accent #8F2412 for pressed / dim accent
  soft accent #F2745A for numeric values and thin line graphs
  ink #EEF1F5 for primary text
  muted #8A929E for secondary text
  NOTHING ELSE. Absolutely no gold, no yellow, no amber, no blue, no green, no teal,
  no purple, no pink.

TYPOGRAPHY
One sans-serif family throughout (Inter / Helvetica-like), all labels UPPERCASE with wide
letterspacing, numbers in tabular figures. Readability is the top priority.
At 1440 px width: module titles 11-12 px, knob labels 10-11 px, knob values 9-10 px,
footer 8-9 px. Nothing smaller than 8 px. Every knob carries a visible name above or
below it and its current value under the name — never an unlabelled dial.
The only non-sans element is the artist's handwritten signature, which appears once,
drawn as a fine vermilion stroke with the word "Milad" above it.

THE PORTRAIT — mandatory, and functional
A portrait photograph of the designer (a man in his thirties, dark hair, short beard,
wearing glasses, lit from the side in deep vermilion against black) must be an ACTUAL
COMPONENT of the interface, not wallpaper and not a small round avatar. Give it a real
job: the column or block that holds the macro controls, the preset, or the signature.
It should be integrated with soft gradient fades into the black background, never a
hard-edged rectangle floating on top.

LAYOUT CONTENT — every element listed must be present, readable and aligned

Header:
  MH monogram, then "MH SIGNATURE" and beneath it "BY MILAD HASHEMI"
  a tab strip: ALL | EQ | DYN | COLOR | SPACE | MOD | IMAGE  (ALL is active, shown as a
  solid accent pill, the others are plain muted text)
  a preset pill reading "SIGNATURE PRESET · 001" above "Modern Clarity"
  top-right, small caps tagline "SOUND WITH INTENTION"

Signature Engine (the hero block, centre or left-centre):
  one large dial labelled INTENSITY 84%
  four smaller dials: CHARACTER 62, DENSITY 48, SPACE 70, WIDTH 56
  the caption "TURN IDEAS INTO SOUND"

Ten module cards, each with a title at top-left and an ON/OFF switch at top-right:
the switch is a small rounded pill containing a filled accent dot and the word ON.
  EQ:        HPF 32 Hz · LOW +3.0 · LO MID -2.4 · HI MID -3.5 · HIGH -4.0   (+ a thin
             EQ curve graph inside the card)
  DYNAMICS:  THRESH -18.0 · RATIO 4.0:1 · ATTACK 10 ms · RELEASE 100 ms · MAKEUP +3.2
             (+ a horizontal gain-reduction bar showing "-6.8 dB")
  VOCAL:     HPF 95 Hz · BODY +2.0 · DE-ESS 6.5 k · PRES -1.5 · AIR -6.0
  COLOR:     DRIVE 4.5 · TONE 37% · MIX 58%  (+ a small chip reading "CONSOLE")
  DELAY:     TIME 375 ms · FDBK 36% · MIX 18%
  REVERB:    DECAY 3.4 s · DAMP 70% · MIX 24%
  IMAGE:     WIDTH 132% · BALANCE 0  (+ a small goniometer arc)
  MODULATION:RATE 0.30 · DEPTH 28% · MIX 16%
  TRANSIENT: ATTACK +12 · SUSTAIN -8 · SPLIT 180 Hz
  I/O:       INPUT -3.2 · OUTPUT -1.0 · MIX 100%

Two vertical level meters with a dB scale from 0 to -60:
  INPUT -3.2 dB on the far left, OUTPUT -1.0 dB on the far right, thin accent bars.

Footer, one line across the bottom:
  "VERSION 1.0.0"  ·  "AN ALL-IN-ONE MIXING ENVIRONMENT"  ·  "DESIGNED BY MILAD HASHEMI"

COMPOSITION RULES
Everything sits on an 8 px spacing grid with equal gutters. Cards are rectangles with a
small radius (5-8 px), aligned to each other, never overlapping. Maximum five knobs in a
row and two rows per card. The accent appears at most three or four times per row of the
screen — an active tab, a few value numbers, a thin arc. Everything else is ink on black.

IMPORTANT — this will be rebuilt in code
So every element must be simple enough to draw with rectangles, circles, 1 px lines and
small text: no photographic textures other than the portrait, no complex illustrations.
The little displays inside cards should be simple single-line visualisations (a curve, a
bar, a decay line, an arc).

Output: one clean, flat, front-facing interface mockup, 1440 x 900, at 2x resolution.
```

---

## Prompt B — short version

For models with a small prompt window (Midjourney, SDXL):

```
Flat minimal UI mockup for an audio mixing plugin, 1440x900, front view. Ten dark
module cards around one big centre dial labelled INTENSITY. Palette: #050607 background,
#101216 panels, 1px #24272D borders, single accent #D0361C, text #EEF1F5. No gold, no
yellow, no blue. Uppercase sans labels with wide tracking, minimum 10px, every knob
named with a value beneath it. Each card has a small ON pill at top right. A vermilion-lit
portrait of a man with glasses is built into the left column as a real UI element.
Header: MH SIGNATURE, tabs ALL EQ DYN COLOR SPACE MOD IMAGE. Footer: AN ALL-IN-ONE
MIXING ENVIRONMENT. Precision hardware feel, Swiss editorial, no 3D, no gloss, no neon.
```

---

## Prompt C — negative prompt

```
gold, yellow, amber, brass, blue, green, teal, purple, pink, neon, rainbow,
glossy plastic, chrome bevel, 3D render, screws, wood, brushed metal texture,
glass reflection, hologram, sci-fi HUD, circuit boards, equaliser bar animation,
tiny unreadable text, lorem ipsum, blurred text, watermark, logo clutter,
rounded bubble buttons, emoji, drop shadows everywhere, vignette, photograph frame,
desk background, laptop mockup, angled perspective, isometric
```

---

## Prompt D — layout modifiers

Append **one** of these lines to Prompt A to steer the archetype:

1. `Composition: one calm four-column grid, all cards the same height, no hero element except the central dial.`
2. `Composition: the portrait forms a full-height left wall; engine macro dials sit on top of the portrait; modules in three columns on the right.`
3. `Composition: no cards at all — ten horizontal rows separated by hairlines, like a ledger; each row holds a title, an ON pill and its knobs.`
4. `Composition: a hard two-column split; left half is portrait + engine + a chain-status list, right half is a two-column module grid.`
5. `Composition: a console — ten vertical channel strips, each with its module name, an ON pill and vertical faders instead of dials.`
6. `Composition: one enormous central dial occupying a third of the screen, four satellite dials around it, and the ten modules as a compact list down the right edge.`
7. `Composition: a single tall column of horizontal module bars; the portrait and analyser live in a narrow rail on the right.`
8. `Composition: the portrait bleeds in from the top-right corner and fades into black; the engine sits top-left; modules fill the lower half in three columns.`
9. `Composition: a blueprint — thin grid lines across the whole surface, cards drawn only as 1px outlines, the accent used for value bars instead of fills.`
10. `Composition: two wide columns of landscape cards; a full-width engine band with the portrait inset runs across the top.`

---

## Prompt E — edit prompts

Use these on an image you already like instead of starting over:

- `Keep the layout exactly. Make every label 30% larger and increase the space between the label and the dial.`
- `Keep everything. Replace the card fills with pure #050607 and keep only the 1px #24272D borders.`
- `Keep the composition. Add the ON/OFF pill to any module card that is missing it.`
- `Keep the layout. Make the portrait larger and let it fade into black on its right edge.`
- `Keep the layout. Reduce the accent to two places only: the active tab and one active control.`
- `Keep the layout. Remove all gradients except the soft glow behind the INTENSITY dial.`
- `Keep the layout. Make the type smaller in the header but leave the module labels as they are.`
- `Keep the layout. Convert the surrounding space to pure black and remove any border around the whole window.`

---

## Prompt F — acceptance checklist

Before sending a concept back, check:

| # | Check | Why it matters |
|---|---|---|
| 1 | **16:10**, whole interface visible, no crop | the plugin window is 1440 × 900 |
| 2 | Every knob has a **readable name** | hard requirement |
| 3 | Every module card has an **ON / OFF switch** | hard requirement |
| 4 | Only **#D0361C** as accent — zero gold/yellow | hard requirement |
| 5 | Portrait is **doing a job**, not decoration | hard requirement |
| 6 | Labels are readable at 100% zoom (≈10 px at 1440 wide) | the previous design failed here |
| 7 | No element is smaller than a fingertip (≈40 px) | resizing must not break it |
| 8 | Every card is aligned to the others, equal gutters | it will be rebuilt on a grid |
| 9 | Nothing needs a texture or illustration to make sense | must be reproducible in code |
| 10 | Footer line present and complete | branding |

---

## Appendix — exact copy deck

Use these strings verbatim so the generated text matches the product:

```
MH          MH SIGNATURE          BY MILAD HASHEMI          SOUND WITH INTENTION
ALL  EQ  DYN  COLOR  SPACE  MOD  IMAGE
SIGNATURE PRESET · 001 / Modern Clarity
SIGNATURE ENGINE   CHARACTER 62   DENSITY 48   SPACE 70   WIDTH 56   INTENSITY 84%
TURN IDEAS INTO SOUND
INPUT -3.2 dB      OUTPUT -1.0 dB
VERSION 1.0.0   ·   AN ALL-IN-ONE MIXING ENVIRONMENT   ·   DESIGNED BY MILAD HASHEMI
```

Module titles, in chain order:

```
01 EQ            02 DYNAMICS       03 VOCAL         04 COLOR       05 DELAY
06 REVERB        07 IMAGE          08 MODULATION    09 TRANSIENT   10 I/O
```

Every module contributes to one chain, in this order:
`EQ → DYNAMICS → VOCAL → COLOR → MODULATION → TRANSIENT → DELAY → REVERB → IMAGE → I/O`
