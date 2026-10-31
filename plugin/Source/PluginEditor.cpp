#include "PluginEditor.h"
#include "BinaryData.h"

namespace mhui
{

//==============================================================================
juce::String formatValue (float v, Fmt f)
{
    switch (f)
    {
        case Fmt::int0:  return juce::String ((int) std::round (v));
        case Fmt::db1:   return juce::String (v, 1) + " dB";
        case Fmt::dbInt: return juce::String ((int) std::round (v)) + " dB";
        case Fmt::pct0:  return juce::String ((int) std::round (v)) + "%";
        case Fmt::ratio: return juce::String (v, 1) + ":1";
        case Fmt::sec1:  return juce::String (v, 1) + " s";
        case Fmt::raw1:  return juce::String (v, 1);

        case Fmt::hz:
            if (v >= 1000.0f) return juce::String (v / 1000.0f, 1) + " k";
            return juce::String ((int) std::round (v)) + " Hz";

        case Fmt::ms:
            if (v >= 1000.0f) return juce::String (v / 1000.0f, 2) + " s";
            return juce::String ((int) std::round (v)) + " ms";
    }

    return juce::String (v, 2);
}

juce::Font uiFont (float size, bool bold, float kerning)
{
    return juce::Font (juce::Font::getDefaultSansSerifFontName(), size,
                       bold ? juce::Font::bold : juce::Font::plain)
             .withExtraKerningFactor (kerning * 0.055f);
}

juce::Font displayFont (float size, bool bold, float kerning)
{
    return uiFont (size, bold, kerning);
}

namespace
{
    float pv (MHSignatureAudioProcessor& p, const char* id)
    {
        if (auto* a = p.apvts.getRawParameterValue (id))
            return a->load();
        return 0.0f;
    }

    /** Draws text scaled down just enough to fit — labels never overlap. */
    void fitText (juce::Graphics& g, const juce::String& str, juce::Rectangle<float> area,
                  float size, juce::Colour colour, juce::Justification just,
                  bool bold = false, float kerning = 1.2f)
    {
        if (str.isEmpty() || area.getWidth() < 4.0f) return;

        auto font = uiFont (size, bold, kerning);
        const float need = font.getStringWidthFloat (str);
        if (need > area.getWidth() && need > 0.0f)
        {
            size *= juce::jmax (0.60f, area.getWidth() / need);
            font = uiFont (size, bold, kerning);
        }

        g.setFont (font);
        g.setColour (colour);
        g.drawText (str, area, just, false);
    }

    /** The design grid, in points.  Same numbers as assets/gui/rack/render_signature.py */
    namespace geo
    {
        inline const juce::Rectangle<int> portrait { 10,  76,  230, 776 };
        inline const juce::Rectangle<int> inBox    { 246, 76,  54,  776 };
        inline const juce::Rectangle<int> outBox   { 1372, 76, 54,  776 };

        inline const juce::Rectangle<int> engine   { 310, 76,  496, 280 };
        inline const juce::Rectangle<int> spectrum { 816, 76,  546, 280 };

        inline const juce::Rectangle<int> eq       { 310, 366, 310, 164 };
        inline const juce::Rectangle<int> dyn      { 630, 366, 372, 164 };
        inline const juce::Rectangle<int> colour   { 1012, 366, 350, 164 };

        inline const juce::Rectangle<int> vocal    { 310, 540, 330, 164 };
        inline const juce::Rectangle<int> reverb   { 650, 540, 242, 164 };
        inline const juce::Rectangle<int> delay    { 902, 540, 228, 164 };
        inline const juce::Rectangle<int> imager   { 1140, 540, 222, 164 };

        inline const juce::Rectangle<int> mod      { 310, 714, 330, 138 };
        inline const juce::Rectangle<int> transient{ 650, 714, 332, 138 };
        inline const juce::Rectangle<int> io       { 992, 714, 370, 138 };

        inline const juce::Rectangle<int> tabBar   { 540, 12, 480, 40 };
        inline const juce::Rectangle<int> preset   { 1124, 13, 226, 42 };
    }
}

//==============================================================================
namespace art
{
    void card (juce::Graphics& g, juce::Rectangle<float> area, float radius, float glow)
    {
        const auto r = area.reduced (0.5f);

        if (glow > 0.01f)
        {
            g.setColour (deepAccent.withAlpha (0.16f * glow));
            g.fillRoundedRectangle (r.expanded (14.0f), radius + 14.0f);
            g.setColour (deepAccent.withAlpha (0.10f * glow));
            g.fillRoundedRectangle (r.expanded (6.0f), radius + 6.0f);
        }

        g.setGradientFill (juce::ColourGradient (cardTop, r.getX(), r.getY(),
                                                 cardBottom, r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, radius);

        g.setColour (edgeCol);
        g.drawRoundedRectangle (r, radius, 1.0f);

        g.setColour (juce::Colour (0xff20242a));
        g.drawLine (r.getX() + radius + 5.0f, r.getY() + 1.0f,
                    r.getRight() - radius - 5.0f, r.getY() + 1.0f, 1.0f);
    }

    void glowArc (juce::Graphics& g, float cx, float cy, float r,
                  float a0, float a1, juce::Colour colour, float width)
    {
        juce::Path halo;
        halo.addCentredArc (cx, cy, r, r, 0.0f, a0, a1, true);
        g.setColour (colour.withAlpha (0.18f));
        g.strokePath (halo, juce::PathStrokeType (width + 5.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        juce::Path p;
        p.addCentredArc (cx, cy, r, r, 0.0f, a0, a1, true);
        g.setColour (colour);
        g.strokePath (p, juce::PathStrokeType (width, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    // ------------------------------------------------------------------ artwork
    static void eqCurve (juce::Graphics& g, juce::Rectangle<float> a,
                         MHSignatureAudioProcessor& proc)
    {
        const float lowG  = pv (proc, Params::eqLowG);
        const float lmG   = pv (proc, Params::eqLmG);
        const float hmG   = pv (proc, Params::eqHmG);
        const float highG = pv (proc, Params::eqHighG);
        const float tilt  = pv (proc, Params::eqTilt);
        const float hpf   = pv (proc, Params::eqHpf);

        const float mid = a.getCentreY();

        g.setColour (juce::Colour (0xff191c22));
        for (int i = 1; i < 4; ++i)
            g.drawLine (a.getX() + a.getWidth() * (float) i / 4.0f, a.getY(),
                        a.getX() + a.getWidth() * (float) i / 4.0f, a.getBottom(), 1.0f);
        g.setColour (juce::Colour (0xff1e2128));
        g.drawLine (a.getX(), mid, a.getRight(), mid, 1.0f);

        auto bell = [] (float f, float fc, float oct)
        {
            const float d = std::log2 (juce::jmax (1.0f, f) / fc) / oct;
            return std::exp (-0.5f * d * d);
        };

        juce::Path p;
        float nodeX[4] {}, nodeY[4] {};
        int node = 0;

        for (int i = 0; i < 110; ++i)
        {
            const float t = (float) i / 109.0f;
            const float f = 20.0f * std::pow (1000.0f, t);
            float db = lowG * bell (f, 65.0f, 1.15f)
                     + lmG  * bell (f, 152.0f, 0.85f)
                     + hmG  * bell (f, 3500.0f, 0.95f)
                     + highG * bell (f, 9000.0f, 1.5f)
                     + tilt * (t - 0.5f) * 6.0f;

            if (f < hpf)
                db -= 10.0f * std::log2 (hpf / juce::jmax (20.0f, f));

            const float x = a.getX() + t * a.getWidth();
            const float y = mid - (db / 14.0f) * (a.getHeight() * 0.46f);

            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);

            if (node < 4 && (i == 10 || i == 28 || i == 58 || i == 84))
            {
                nodeX[node] = x;
                nodeY[node] = y;
                ++node;
            }
        }

        g.setColour (softAccent);
        g.strokePath (p, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

        for (int i = 0; i < juce::jmin (node, 4); ++i)
        {
            g.setColour (juce::Colour (0xfffff5f2));
            g.fillEllipse (nodeX[i] - 2.6f, nodeY[i] - 2.6f, 5.2f, 5.2f);
            g.setColour (accent);
            g.drawEllipse (nodeX[i] - 2.6f, nodeY[i] - 2.6f, 5.2f, 5.2f, 1.0f);
        }
    }

    static void compCurve (juce::Graphics& g, juce::Rectangle<float> a,
                           MHSignatureAudioProcessor& proc)
    {
        auto left = a.withWidth (a.getWidth() * 0.56f);

        g.setColour (juce::Colour (0xff090a0d));
        g.fillRoundedRectangle (left, 4.0f);
        g.setColour (juce::Colour (0xff1b1e24));
        g.drawRoundedRectangle (left, 4.0f, 1.0f);

        for (int i = 1; i < 4; ++i)
        {
            g.setColour (juce::Colour (0xff171a1f));
            g.drawLine (left.getX(), left.getY() + left.getHeight() * (float) i / 4.0f,
                        left.getRight(), left.getY() + left.getHeight() * (float) i / 4.0f, 1.0f);
        }

        juce::Path p;
        for (int i = 0; i < 40; ++i)
        {
            const float t = (float) i / 39.0f;
            const float x = left.getX() + 4.0f + (left.getWidth() - 8.0f) * std::pow (t, 0.62f);
            const float y = left.getBottom() - 3.0f - (left.getHeight() - 6.0f) * t;
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (accent);
        g.strokePath (p, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

        const float gx = a.getX() + a.getWidth() * 0.68f;
        fitText (g, "GR", { gx, a.getY() - 1.0f, 26.0f, 10.0f }, 7.0f, dim,
                 juce::Justification::centredLeft, true, 1.2f);

        auto bar = juce::Rectangle<float> (gx, a.getY() + 11.0f, a.getRight() - gx,
                                           juce::jmax (8.0f, a.getHeight() - 14.0f));
        g.setColour (juce::Colour (0xff090a0d));
        g.fillRoundedRectangle (bar, 3.0f);
        g.setColour (juce::Colour (0xff1b1e24));
        g.drawRoundedRectangle (bar, 3.0f, 1.0f);

        const float gr = proc.getGainReductionDb();
        const float amount = juce::jlimit (0.0f, 1.0f, -gr / 16.0f);
        if (amount > 0.01f)
        {
            g.setColour (accent);
            g.fillRoundedRectangle (bar.reduced (1.5f).withWidth (
                juce::jmax (3.0f, (bar.getWidth() - 3.0f) * amount)), 2.0f);
        }
    }

    static void saturationCurve (juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float mid = a.getCentreY();
        juce::Path p;
        for (int i = 0; i < 90; ++i)
        {
            const float t = (float) i / 89.0f;
            const float x = a.getX() + t * a.getWidth();
            const float y = mid - (a.getHeight() * 0.40f)
                                 * std::tanh (std::sin (t * 7.0f * juce::MathConstants<float>::pi)
                                              * 2.8f);
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (softAccent);
        g.strokePath (p, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
        g.setColour (juce::Colour (0xff1a1d23));
        g.drawLine (a.getX(), a.getBottom() + 1.0f, a.getRight(), a.getBottom() + 1.0f, 1.0f);
    }

    static void decayCurve (juce::Graphics& g, juce::Rectangle<float> a, float rate)
    {
        juce::Path p;
        for (int i = 0; i < 60; ++i)
        {
            const float t = (float) i / 59.0f;
            const float x = a.getX() + t * a.getWidth();
            const float y = a.getBottom() - 2.0f - std::exp (-t * rate) * (a.getHeight() - 4.0f);
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (softAccent);
        g.strokePath (p, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
        g.setColour (juce::Colour (0xff1c1f26));
        g.drawLine (a.getX(), a.getBottom(), a.getRight(), a.getBottom(), 1.0f);
    }

    static void delayDots (juce::Graphics& g, juce::Rectangle<float> a,
                           MHSignatureAudioProcessor& proc)
    {
        const float fb = juce::jlimit (0.0f, 1.0f, pv (proc, Params::dlyFb) * 0.01f);

        for (int c = 0; c < 9; ++c)
        {
            const float amp = juce::jmax (0.06f, std::pow (0.62f, (float) c));
            const float x = a.getX() + a.getWidth() * (float) c / 8.0f;

            for (int r = 0; r < 2; ++r)
            {
                const float y = a.getY() + a.getHeight() * (0.32f + 0.38f * (float) r);
                const float rad = 1.8f + 1.7f * amp * (0.4f + 0.6f * fb);
                g.setColour (softAccent.withAlpha (juce::jlimit (0.05f, 0.95f, amp * 1.6f)));
                g.fillEllipse (x - rad, y - rad, rad * 2.0f, rad * 2.0f);
            }
        }
    }

    static void lfoCurve (juce::Graphics& g, juce::Rectangle<float> a)
    {
        juce::Path p;
        for (int i = 0; i < 80; ++i)
        {
            const float t = (float) i / 79.0f;
            const float x = a.getX() + t * a.getWidth();
            const float y = a.getCentreY() - a.getHeight() * 0.40f
                                * std::sin (t * 4.0f * juce::MathConstants<float>::twoPi);
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (softAccent);
        g.strokePath (p, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    static void imagerFan (juce::Graphics& g, juce::Rectangle<float> a,
                           MHSignatureAudioProcessor& proc)
    {
        const float width = juce::jlimit (0.0f, 2.0f, pv (proc, Params::imgWidth) * 0.01f);
        const float bal = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::imgBalance) * 0.01f);

        const float cx = a.getCentreX();
        const float cy = a.getBottom();
        const float r = juce::jmin (a.getWidth() * 0.42f, a.getHeight() * 1.05f);

        juce::Path base;
        base.addCentredArc (cx, cy, r, r, 0.0f, juce::MathConstants<float>::pi,
                            juce::MathConstants<float>::twoPi, true);
        g.setColour (juce::Colour (0xff2a2e36));
        g.strokePath (base, juce::PathStrokeType (1.2f));

        const float spread = juce::jlimit (0.10f, 1.0f, width * 0.5f) * 0.9f;
        const float centre = -juce::MathConstants<float>::halfPi + bal * 0.4f;

        for (int i = -6; i <= 6; ++i)
        {
            const float ang = centre + spread * (float) i / 6.0f;
            const bool lit = std::abs (i) <= 2;

            const float x0 = cx + std::cos (ang) * r * 0.55f;
            const float y0 = cy + std::sin (ang) * r * 0.55f;
            const float x1 = cx + std::cos (ang) * r * 0.94f;
            const float y1 = cy + std::sin (ang) * r * 0.94f;

            g.setColour (lit ? accent.withAlpha (0.85f) : juce::Colour (0xff4c4a4e));
            g.drawLine (x0, y0, x1, y1, lit ? 1.3f : 1.0f);
        }
    }

    static void transientCurve (juce::Graphics& g, juce::Rectangle<float> a,
                                MHSignatureAudioProcessor& proc)
    {
        const float att = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::trnAtt) * 0.01f);
        const float sus = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::trnSus) * 0.01f);

        juce::Path p;
        for (int i = 0; i < 60; ++i)
        {
            const float t = (float) i / 59.0f;
            const float env = std::exp (-t * 5.0f) * (0.58f + 0.24f * att)
                            + (1.0f - std::exp (-t * 12.0f)) * 0.26f * (1.0f + sus);
            const float x = a.getX() + t * a.getWidth();
            const float y = a.getBottom() - 2.0f
                          - juce::jlimit (0.0f, 1.0f, env) * (a.getHeight() - 4.0f);
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (softAccent);
        g.strokePath (p, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
        g.setColour (juce::Colour (0xff1c1f26));
        g.drawLine (a.getX(), a.getBottom(), a.getRight(), a.getBottom(), 1.0f);
    }

    static void ioRails (juce::Graphics& g, juce::Rectangle<float> a)
    {
        for (int i = 1; i < 4; ++i)
        {
            g.setColour (juce::Colour (0xff171a20));
            g.drawLine (a.getX(), a.getY() + a.getHeight() * (float) i / 4.0f,
                        a.getRight(), a.getY() + a.getHeight() * (float) i / 4.0f, 1.0f);
        }
        g.setColour (juce::Colour (0xff262a31));
        g.drawLine (a.getX(), a.getCentreY(), a.getRight(), a.getCentreY(), 1.0f);
    }
}

//==============================================================================
MhKnob::MhKnob (MHSignatureAudioProcessor& proc, const juce::String& paramId,
                const juce::String& labelText, Fmt format, bool bigStyle, bool hideText)
    : juce::Slider (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox),
      knobLabel (labelText), fmt (format), big (bigStyle), noText (hideText)
{
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setDoubleClickReturnValue (true, 0.0);
    setWantsKeyboardFocus (false);

    param = proc.apvts.getParameter (paramId);

    if (param != nullptr)
    {
        const auto range = param->getNormalisableRange();
        setRange (range.start, range.end, 0.0);
        setSkewFactor (range.skew);
        setValue (range.convertFrom0to1 (param->getValue()), juce::dontSendNotification);

        // the value shown comes straight from the parameter, so a preset change
        // updates the read-out even while the knob is not being touched
        setTooltip (param->getName (40));
    }

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, paramId, *this);

    onValueChange = [this] { repaint(); };
}

void MhKnob::setAccentGlow (float g)
{
    const float next = juce::jlimit (0.0f, 1.0f, g);
    if (std::abs (next - glow) > 0.02f) { glow = next; repaint(); }
}

void MhKnob::setDimmed (bool shouldDim) { dimmed = shouldDim; repaint(); }

void MhKnob::mouseEnter (const juce::MouseEvent&) { hovered = true;  repaint(); }
void MhKnob::mouseExit  (const juce::MouseEvent&) { hovered = false; repaint(); }

void MhKnob::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    if (b.getWidth() < 14.0f || b.getHeight() < 14.0f) return;

    const float textH = noText ? 2.0f : (big ? 34.0f : 27.0f);
    const float d = juce::jmin (b.getWidth() - 2.0f, b.getHeight() - textH);
    if (d < 18.0f) return;

    const float cx = b.getCentreX();
    const float cy = b.getY() + juce::jmax (0.0f, (b.getHeight() - textH - d) * 0.5f) + d * 0.5f;
    const float r  = d * 0.5f - 1.0f;

    const float a0 = juce::MathConstants<float>::pi * 1.25f;
    const float a1 = juce::MathConstants<float>::pi * 2.75f;
    const float sweep = a1 - a0;
    const float norm = (float) valueToProportionOfLength (getValue());
    const float stroke = big ? 3.4f : 2.2f;

    g.setOpacity (dimmed ? 0.42f : 1.0f);

    juce::Path track;
    track.addCentredArc (cx, cy, r, r, 0.0f, a0, a1, true);
    g.setColour (trackCol);
    g.strokePath (track, juce::PathStrokeType (stroke, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    if (norm > 0.004f)
    {
        art::glowArc (g, cx, cy, r, a0, a0 + sweep * norm,
                      hovered ? softAccent : accent, stroke);
    }

    // brushed cap
    const float bodyR = r - (big ? 8.0f : 7.0f);
    const juce::Rectangle<float> body (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1e2126), cx, cy - bodyR,
                                             juce::Colour (0xff0c0d10), cx, cy + bodyR, false));
    g.fillEllipse (body);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0x33ffffff), cx - bodyR * 0.2f,
                                             cy - bodyR * 0.92f, juce::Colours::transparentBlack,
                                             cx + bodyR * 0.25f, cy + bodyR * 0.15f, true));
    g.fillEllipse (body);

    g.setColour (juce::Colour (0x24ffffff));
    g.drawEllipse (body.reduced (0.5f), 1.0f);

    const float ang = a0 + sweep * norm;
    const float r0 = bodyR * 0.26f;
    const float r1 = bodyR * 0.88f;
    const juce::Point<float> p0 (cx + std::cos (ang) * r0, cy + std::sin (ang) * r0);
    const juce::Point<float> p1 (cx + std::cos (ang) * r1, cy + std::sin (ang) * r1);

    g.setColour (juce::Colour (0x90000000));
    g.drawLine (p0.x + 1.0f, p0.y + 1.0f, p1.x + 1.0f, p1.y + 1.0f, big ? 3.0f : 2.2f);

    g.setColour (hovered ? softAccent : juce::Colour (0xffe9ecef));
    g.drawLine (p0.x, p0.y, p1.x, p1.y, big ? 2.8f : 2.0f);

    if (noText) { g.setOpacity (1.0f); return; }

    fitText (g, knobLabel,
             { b.getX() - 6.0f, cy + r + 0.5f, b.getWidth() + 12.0f, 13.0f },
             typeLabel, juce::Colour (0xffd2d8df), juce::Justification::centred,
             true, 1.5f);

    fitText (g, formatValue ((float) getValue(), fmt),
             { b.getX() - 6.0f, cy + r + 13.0f, b.getWidth() + 12.0f, 12.0f },
             typeValue, softAccent, juce::Justification::centred, false, 0.3f);

    g.setOpacity (1.0f);
}

//==============================================================================
PowerSwitch::PowerSwitch (MHSignatureAudioProcessor& proc,
                          const juce::String& paramId,
                          const juce::String& shortName)
    : juce::Button (paramId), moduleName (shortName)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setTooltip (shortName + ": click to take this module in or out of the chain");
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        proc.apvts, paramId, *this);

    onClick = [this] { if (onToggle) onToggle(); repaint(); };
}

void PowerSwitch::paintButton (juce::Graphics& g, bool over, bool down)
{
    const bool on = getToggleState();

    auto r = getLocalBounds().toFloat().reduced (0.5f);
    const float corner = r.getHeight() * 0.5f;

    if (on)
    {
        g.setColour (deepAccent.withAlpha (0.30f));
        g.fillRoundedRectangle (r.expanded (2.5f), corner + 2.5f);
    }

    g.setColour (on ? juce::Colour (0xff1e100d) : juce::Colour (0xff111317));
    g.fillRoundedRectangle (r, corner);

    g.setColour (on ? accent : (over ? juce::Colour (0xff5e6672) : juce::Colour (0xff2f343c)));
    g.drawRoundedRectangle (r, corner, 1.0f);

    // the lamp
    const float dotR = r.getHeight() * 0.30f;
    const float dotX = r.getX() + r.getHeight() * 0.5f;
    const float dotY = r.getCentreY();

    if (on)
    {
        g.setColour (accent.withAlpha (0.30f));
        g.fillEllipse (dotX - dotR - 1.6f, dotY - dotR - 1.6f,
                       (dotR + 1.6f) * 2.0f, (dotR + 1.6f) * 2.0f);
        g.setColour (down ? deepAccent : accent);
        g.fillEllipse (dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
    }
    else
    {
        g.setColour (juce::Colour (0xff1a1d22));
        g.fillEllipse (dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
        g.setColour (juce::Colour (0xff424852));
        g.drawEllipse (dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f, 1.0f);
    }

    fitText (g, on ? "ON" : "OFF",
             r.withTrimmedLeft (r.getHeight() + 1.0f).withTrimmedRight (5.0f),
             7.2f, on ? juce::Colour (0xfff2a08c) : (over ? ink2 : dim),
             juce::Justification::centredLeft, true, 1.4f);
}

//==============================================================================
ModulePanel::ModulePanel (MHSignatureAudioProcessor& p, const juce::String& t,
                          bool showSwitch)
    : title (t), proc (p), hasSwitch (showSwitch)
{
    setInterceptsMouseClicks (false, true);
}

void ModulePanel::addPower (const juce::String& paramId, const juce::String& shortName)
{
    auto s = std::make_unique<PowerSwitch> (proc, paramId, shortName);
    s->onToggle = [this] { repaint(); };
    addAndMakeVisible (*s);
    powers.push_back (std::move (s));
    resized();
}

void ModulePanel::addKnob (const juce::String& id, const juce::String& label, Fmt fmt,
                           bool accentPointer)
{
    juce::ignoreUnused (accentPointer);

    auto k = std::make_unique<MhKnob> (proc, id, label, fmt);
    addAndMakeVisible (*k);
    k->onValueChange = [this] { repaint (getBodyArea()); };
    knobs.push_back (std::move (k));
    resized();
}

void ModulePanel::setArt (std::function<void (juce::Graphics&, juce::Rectangle<float>)> drawer)
{
    artDrawer = std::move (drawer);
    resized();
    repaint();
}

void ModulePanel::setReadout (juce::String text) { readout = std::move (text); repaint(); }

void ModulePanel::setCaption (juce::String text) { caption = std::move (text); repaint(); }

void ModulePanel::setGlow (float g) { glowAmount = g; repaint(); }

juce::Rectangle<int> ModulePanel::headerArea() const
{
    return getLocalBounds().reduced (13, 0).withHeight (31);
}

juce::Rectangle<int> ModulePanel::getBodyArea() const
{
    auto r = getLocalBounds().reduced (13, 0);
    r.removeFromTop (32);
    r.removeFromBottom (6);
    return r;
}

juce::Rectangle<int> ModulePanel::getArtArea() const
{
    auto body = getBodyArea();
    const int n = (int) knobs.size();
    if (n == 0) return body;

    const int slot = body.getWidth() / juce::jmax (1, n);
    const int d = juce::jlimit (38, 56, slot - 12);
    const int block = d + 30;

    body.removeFromBottom (block + 7);
    return body.withTrimmedBottom (4);
}

bool ModulePanel::isAnyModuleOn() const
{
    if (powers.empty()) return true;
    for (const auto& s : powers)
        if (s->getToggleState()) return true;
    return false;
}

void ModulePanel::refreshDimming() { setFocused (focused); }

void ModulePanel::setFocused (bool shouldBeFocused)
{
    focused = shouldBeFocused;

    const bool live = isAnyModuleOn();
    for (auto& k : knobs) k->setDimmed (! focused || ! live);

    repaint();
}

void ModulePanel::resized()
{
    for (size_t i = 0; i < powers.size(); ++i)
        powers[i]->setBounds (13 + (int) i * 52, 7, 48, 18);

    if (customLayout) customLayout();

    const int n = (int) knobs.size();
    if (n == 0) return;

    auto body = getBodyArea();
    if (body.getHeight() < 30 || body.getWidth() < 30) return;

    const int slot = body.getWidth() / n;
    const int d = juce::jlimit (38, 56, slot - 12);
    const int block = d + 30;
    const int top = getHeight() - 7 - block;

    for (int i = 0; i < n; ++i)
        knobs[(size_t) i]->setBounds (body.getX() + i * slot, top, slot, block);
}

void ModulePanel::paint (juce::Graphics& g)
{
    const bool live = isAnyModuleOn();
    const float alpha = focused ? 1.0f : 0.42f;

    g.setOpacity (alpha);
    art::card (g, getLocalBounds().toFloat(), 8.0f, glowAmount);

    const float tx = 13.0f + 52.0f * (float) powers.size();

    const float titleW = (float) getWidth() - tx - (readout.isNotEmpty() ? 158.0f : 14.0f);

    fitText (g, title.toUpperCase(), { tx, 6.0f, titleW, 20.0f },
             typeModule, live ? ink : muted, juce::Justification::centredLeft, true, 2.2f);

    if (readout.isNotEmpty())
    {
        fitText (g, readout.toUpperCase(),
                 { (float) getWidth() - 152.0f, 6.0f, 138.0f, 20.0f },
                 7.8f, dim, juce::Justification::centredRight, true, 1.2f);
    }

    g.setColour (juce::Colour (0xff191c22));
    g.drawLine (13.0f, 31.5f, (float) getWidth() - 13.0f, 31.5f, 1.0f);

    if (artDrawer != nullptr)
    {
        auto a = getArtArea();
        if (a.getHeight() > 14)
            artDrawer (g, a.toFloat());
    }

    if (caption.isNotEmpty())
        fitText (g, caption, { 13.0f, (float) getHeight() - 18.0f, (float) getWidth() - 26.0f, 14.0f },
                 7.4f, dim, juce::Justification::centred, true, 2.6f);

    g.setOpacity (1.0f);

    if (! live)
    {
        g.setColour (juce::Colour (0x66050607));
        g.fillRoundedRectangle (getLocalBounds().toFloat(), 8.0f);
    }
}

//==============================================================================
ChoiceChip::ChoiceChip (MHSignatureAudioProcessor& p, const juce::String& paramId,
                        const juce::String& cap)
    : proc (p), caption (cap)
{
    param = proc.apvts.getParameter (paramId);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setTooltip ("Click to change character");
}

void ChoiceChip::paint (juce::Graphics& g)
{
    if (param == nullptr) return;

    auto r = getLocalBounds().toFloat();
    const juce::String value = param->getText (param->getValue(), 14).toUpperCase();
    const bool on = true;

    g.setColour (on ? juce::Colour (0xff1e100d) : juce::Colour (0xff101216));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (on ? accent : juce::Colour (0xff2a2e35));
    g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.0f);

    fitText (g, caption, r.withTrimmedRight (r.getWidth() - 34.0f), 6.2f, dim,
             juce::Justification::centredLeft, true, 1.6f);

    fitText (g, value, r.withTrimmedLeft (34.0f).withTrimmedRight (6.0f), 6.8f,
             juce::Colour (0xfff2a08c), juce::Justification::centredRight, true, 1.2f);
}

void ChoiceChip::mouseUp (const juce::MouseEvent&)
{
    if (param == nullptr) return;

    const int steps = param->getNumSteps();
    const int current = (int) std::round (param->getValue() * (float) (steps - 1));
    param->setValueNotifyingHost ((float) ((current + 1) % steps) / (float) (steps - 1));
    repaint();
}

//==============================================================================
LevelMeter::LevelMeter (MHSignatureAudioProcessor& p, bool inputSide)
    : proc (p), isInput (inputSide)
{
    setInterceptsMouseClicks (false, false);
}

void LevelMeter::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    art::card (g, b, 8.0f);

    const float cx = b.getCentreX();

    fitText (g, isInput ? "INPUT" : "OUTPUT",
             { b.getX(), b.getY() + 8.0f, b.getWidth(), 16.0f },
             7.8f, ink2, juce::Justification::centred, true, 2.4f);

    const float top = b.getY() + 34.0f;
    const float bot = b.getBottom() - 32.0f;

    const float barW = 8.0f;
    const float barX = cx - barW * 0.5f;

    g.setColour (juce::Colour (0xff08090c));
    g.fillRoundedRectangle (barX - 4.0f, top - 2.0f, barW + 8.0f, bot - top + 4.0f, 5.0f);
    g.setColour (juce::Colour (0xff1c1f26));
    g.drawRoundedRectangle (barX - 4.0f, top - 2.0f, barW + 8.0f, bot - top + 4.0f, 5.0f, 1.0f);

    const float db = isInput ? proc.getInputLevelDb() : proc.getOutputLevelDb();

    for (int step = 0; step >= -60; step -= 6)
    {
        const float y = bot - ((float) step + 60.0f) / 60.0f * (bot - top);

        g.setColour (juce::Colour (0xff3a4048));
        g.drawLine (barX - 7.0f, y, barX - 4.0f, y, 1.0f);

        fitText (g, juce::String (step),
                 isInput ? juce::Rectangle<float> (barX + barW + 3.0f, y - 6.0f, 19.0f, 12.0f)
                         : juce::Rectangle<float> (barX - 22.0f, y - 6.0f, 19.0f, 12.0f),
                 6.2f, dim,
                 isInput ? juce::Justification::centredLeft : juce::Justification::centredRight,
                 false, 0.2f);
    }

    const float n = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
    if (n > 0.002f)
    {
        const float y0 = bot - n * (bot - top);

        g.setColour (deepAccent.withAlpha (0.30f));
        g.fillRoundedRectangle (barX - 2.0f, y0 - 1.0f, barW + 4.0f, bot - y0 + 2.0f, 2.0f);

        for (float y = bot; y > y0; y -= 4.1f)
        {
            const float segTop = juce::jmax (y0, y - 2.6f);
            const float h = juce::jmax (0.6f, y - 0.7f - segTop);
            const float frac = (bot - y) / juce::jmax (1.0f, bot - top);
            g.setColour (frac > 0.78f ? softAccent : accent);
            g.fillRect (barX, segTop, barW, h);
        }

        g.setColour (softAccent);
        g.fillRect (barX - 3.5f, y0 - 1.0f, barW + 7.0f, 1.4f);
    }

    fitText (g, db <= -99.0f ? "-inf dB" : juce::String (db, 1) + " dB",
             { b.getX(), b.getBottom() - 24.0f, b.getWidth(), 16.0f },
             8.0f, ink2, juce::Justification::centred, false, 0.4f);
}

//==============================================================================
TabStrip::TabStrip()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

juce::Rectangle<float> TabStrip::tabBounds (int i) const
{
    const float widths[] { 44.0f, 40.0f, 42.0f, 60.0f, 62.0f, 46.0f, 66.0f };
    const float gap = 20.0f;

    float total = 0.0f;
    for (int k = 0; k < 7; ++k) total += widths[k];
    total += gap * 6.0f;

    float x = (float) getWidth() * 0.5f - total * 0.5f;
    for (int k = 0; k < i; ++k) x += widths[k] + gap;

    return { x, 0.0f, widths[i], (float) getHeight() };
}

void TabStrip::paint (juce::Graphics& g)
{
    for (int i = 0; i < items.size(); ++i)
    {
        auto tb = tabBounds (i);
        const bool active = i == current;

        fitText (g, items[i], tb, 8.8f,
                 active ? ink : (i == hovered ? ink2 : muted),
                 juce::Justification::centred, true, 2.0f);

        if (active)
        {
            const float y = tb.getCentreY() + 15.0f;
            g.setColour (accent.withAlpha (0.22f));
            g.fillRoundedRectangle (tb.getX(), y - 2.0f, tb.getWidth(), 5.0f, 2.5f);
            g.setColour (accent);
            g.fillRoundedRectangle (tb.getX(), y - 1.0f, tb.getWidth(), 1.8f, 0.9f);
        }
    }
}

void TabStrip::mouseMove (const juce::MouseEvent& e)
{
    int h = -1;
    for (int i = 0; i < items.size(); ++i)
        if (tabBounds (i).contains (e.position)) h = i;

    if (h != hovered) { hovered = h; repaint(); }
}

void TabStrip::mouseExit (const juce::MouseEvent&) { hovered = -1; repaint(); }

void TabStrip::mouseUp (const juce::MouseEvent& e)
{
    for (int i = 0; i < items.size(); ++i)
    {
        if (tabBounds (i).contains (e.position))
        {
            current = i;
            repaint();
            if (onTabChanged) onTabChanged (current);
            return;
        }
    }
}

//==============================================================================
PresetBar::PresetBar (MHSignatureAudioProcessor& p) : proc (p)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setTooltip ("Click to browse the 100 factory presets — wheel to step");
}

void PresetBar::mouseMove (const juce::MouseEvent& e)
{
    const int z = e.position.x < 26.0f ? -1
                                       : (e.position.x > (float) getWidth() - 26.0f ? 1 : 0);
    if (z != hoverZone) { hoverZone = z; repaint(); }
}

void PresetBar::mouseExit (const juce::MouseEvent&) { hoverZone = 0; repaint(); }

void PresetBar::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (juce::Colour (0xff0b0c0f));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (juce::Colour (0xff262a31));
    g.drawRoundedRectangle (r, 6.0f, 1.0f);

    fitText (g, "SIGNATURE PRESET",
             { r.getX() + 12.0f, r.getY() + 6.0f, r.getWidth() - 44.0f, 12.0f },
             6.8f, dim, juce::Justification::centredLeft, true, 1.8f);

    juce::String name = proc.getCurrentPresetName();
    if (proc.isPresetEdited()) name += "  *";

    fitText (g, name,
             { r.getX() + 12.0f, r.getY() + 19.0f, r.getWidth() - 44.0f, 18.0f },
             11.4f, ink2, juce::Justification::centredLeft, false, 0.1f);

    // chevron
    const float cxp = r.getRight() - 18.0f;
    const float cyp = r.getCentreY();
    juce::Path chev;
    chev.startNewSubPath (cxp - 4.0f, cyp - 2.5f);
    chev.lineTo (cxp, cyp + 2.0f);
    chev.lineTo (cxp + 4.0f, cyp - 2.5f);
    g.setColour (hoverZone != 0 ? ink2 : muted);
    g.strokePath (chev, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
}

void PresetBar::mouseUp (const juce::MouseEvent& e)
{
    if (e.position.x < 26.0f)                        { proc.prevPreset(); repaint(); }
    else if (e.position.x > (float) getWidth() - 26.0f) { proc.nextPreset(); repaint(); }
    else                                             { showBrowser(); }
}

void PresetBar::showBrowser()
{
    const auto& presets = getFactoryPresets();

    juce::PopupMenu root;
    juce::String lastCat;
    juce::PopupMenu sub;

    auto flush = [&]
    {
        if (lastCat.isNotEmpty() && sub.getNumItems() > 0)
            root.addSubMenu (lastCat.toUpperCase(), sub);
        sub = juce::PopupMenu();
    };

    for (int i = 0; i < (int) presets.size(); ++i)
    {
        if (presets[(size_t) i].category != lastCat)
        {
            flush();
            lastCat = presets[(size_t) i].category;
        }
        sub.addItem (i + 1, presets[(size_t) i].name, true, i == proc.getCurrentPresetIndex());
    }
    flush();

    auto* bar = this;
    root.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                        [bar] (int result)
                        {
                            if (result > 0) bar->proc.applyPreset (result - 1);
                            bar->repaint();
                        });
}

//==============================================================================
SpectrumPanel::SpectrumPanel (MHSignatureAudioProcessor& p)
    : proc (p)
{
    setInterceptsMouseClicks (true, true);

    for (auto& v : hist) v = -90.0f;
    startTimerHz (24);
}

SpectrumPanel::~SpectrumPanel() = default;

juce::Rectangle<float> SpectrumPanel::tabRect (int i) const
{
    const float widths[] { 84.0f, 88.0f, 84.0f };
    float x = 15.0f;
    for (int k = 0; k < i; ++k) x += widths[k];
    return { x, 6.0f, widths[i], 20.0f };
}

juce::Rectangle<float> SpectrumPanel::chipRect (int i) const
{
    const float w = (float) getWidth();
    return i == 0 ? juce::Rectangle<float> (w - 74.0f, 12.0f, 9.0f, 9.0f)
                  : juce::Rectangle<float> (w - 36.0f, 12.0f, 9.0f, 9.0f);
}

juce::Rectangle<float> SpectrumPanel::plotArea() const
{
    return { 13.0f, 40.0f, (float) getWidth() - 26.0f, (float) getHeight() - 52.0f };
}

void SpectrumPanel::timerCallback()
{
    proc.getSpectrum (spec.data(), false);
    proc.getSpectrum (specPre.data(), true);
    proc.getWaveform (wave.data());

    loudAccum = juce::jmax (loudAccum * 0.93f, proc.getOutputLevelDb());
    if (++loudCounter >= 2)
    {
        loudCounter = 0;
        hist[(size_t) histIndex] = loudAccum;
        histIndex = (histIndex + 1) % (int) hist.size();
    }

    repaint();
}

void SpectrumPanel::paint (juce::Graphics& g)
{
    art::card (g, getLocalBounds().toFloat(), 8.0f);

    for (int i = 0; i < tabNames.size(); ++i)
    {
        auto tb = tabRect (i);

        fitText (g, tabNames[i], tb, 8.2f,
                 i == mode ? ink : dim, juce::Justification::centredLeft, true, 1.8f);

        if (i == mode)
        {
            g.setColour (accent);
            g.fillRoundedRectangle (tb.getX() - 2.0f, tb.getBottom() - 3.0f,
                                    tb.getWidth() - 18.0f, 1.4f, 0.7f);
        }
    }

    // pre / post lamps
    {
        auto preChip = chipRect (0);
        g.setColour (pre ? accent.withAlpha (0.28f) : juce::Colours::transparentBlack);
        g.fillEllipse (preChip.expanded (2.0f));
        g.setColour (pre ? accent : juce::Colour (0xff15181d));
        g.fillEllipse (preChip);
        g.setColour (pre ? accent : juce::Colour (0xff4a505a));
        g.drawEllipse (preChip, 1.0f);

        fitText (g, "PRE", { preChip.getRight() + 5.0f, 9.0f, 26.0f, 14.0f }, 7.0f,
                 pre ? ink2 : dim, juce::Justification::centredLeft, true, 1.5f);

        auto postChip = chipRect (1);
        g.setColour (! pre ? accent : juce::Colour (0xff15181d));
        g.fillEllipse (postChip);
        g.setColour (! pre ? accent : juce::Colour (0xff4a505a));
        g.drawEllipse (postChip, 1.0f);

        fitText (g, "POST", { postChip.getRight() + 5.0f, 9.0f, 30.0f, 14.0f }, 7.0f,
                 ! pre ? ink2 : dim, juce::Justification::centredLeft, true, 1.5f);
    }

    g.setColour (juce::Colour (0xff191c22));
    g.drawLine (13.0f, 32.0f, (float) getWidth() - 13.0f, 32.0f, 1.0f);

    auto plot = plotArea();
    g.setColour (juce::Colour (0xff08090c));
    g.fillRoundedRectangle (plot, 5.0f);

    auto inner = plot.reduced (1.0f).withTrimmedLeft (26.0f).withTrimmedRight (6.0f)
                     .withTrimmedTop (7.0f).withTrimmedBottom (17.0f);

    auto dbToY = [&] (float db)
    {
        const float n = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
        return inner.getBottom() - n * inner.getHeight();
    };
    auto freqToX = [&] (float f)
    {
        const float t = juce::jlimit (0.0f, 1.0f, std::log (juce::jmax (20.0f, f) / 20.0f)
                                                    / std::log (1000.0f));
        return inner.getX() + t * inner.getWidth();
    };

    for (int db = 0; db >= -60; db -= 12)
    {
        const float y = dbToY ((float) db);
        g.setColour (gridCol);
        g.drawLine (inner.getX(), y, inner.getRight(), y, 1.0f);
        fitText (g, juce::String (db), { plot.getX() + 3.0f, y - 6.0f, 20.0f, 12.0f }, 6.2f,
                 juce::Colour (0xff4e545e), juce::Justification::centredRight, false, 0.2f);
    }

    const float freqs[]  { 20.0f, 100.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f, 20000.0f };
    const char* labels[] { "20", "100", "500", "1k", "5k", "10k", "20k" };

    for (int i = 0; i < 7; ++i)
    {
        const float x = freqToX (freqs[i]);
        g.setColour (gridCol);
        g.drawLine (x, inner.getY(), x, inner.getBottom(), 1.0f);
        fitText (g, labels[i], { x - 16.0f, plot.getBottom() - 14.0f, 32.0f, 12.0f }, 6.2f,
                 juce::Colour (0xff4e545e), juce::Justification::centred, false, 0.2f);
    }

    if (mode == 0)
    {
        const auto& data = pre ? specPre : spec;

        juce::Path line, fill;
        for (int i = 0; i < (int) data.size(); ++i)
        {
            const float f = 20.0f * std::pow (1000.0f, ((float) i + 0.5f) / (float) data.size());
            const float x = freqToX (f);
            const float y = dbToY (data[(size_t) i]);

            if (i == 0) { line.startNewSubPath (x, y); fill.startNewSubPath (x, inner.getBottom()); }
            line.lineTo (x, y);
            fill.lineTo (x, y);
        }
        fill.lineTo (inner.getRight(), inner.getBottom());
        fill.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.30f), 0.0f, inner.getY(),
                                                 accent.withAlpha (0.01f), 0.0f, inner.getBottom(),
                                                 false));
        g.fillPath (fill);

        g.setColour (softAccent.withAlpha (0.95f));
        g.strokePath (line, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }
    else if (mode == 1)
    {
        juce::Path p;
        const int n = (int) wave.size();
        for (int i = 0; i < n; ++i)
        {
            const float x = inner.getX() + inner.getWidth() * (float) i / (float) (n - 1);
            const float y = inner.getCentreY() - juce::jlimit (-1.0f, 1.0f, wave[(size_t) i])
                                                 * inner.getHeight() * 0.46f;
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (softAccent.withAlpha (0.9f));
        g.strokePath (p, juce::PathStrokeType (1.2f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }
    else
    {
        juce::Path p;
        const int n = (int) hist.size();
        for (int i = 0; i < n; ++i)
        {
            const float v = hist[(size_t) ((histIndex + i) % n)];
            const float x = inner.getX() + inner.getWidth() * (float) i / (float) (n - 1);
            const float y = dbToY (v);
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (softAccent.withAlpha (0.9f));
        g.strokePath (p, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    g.setColour (juce::Colour (0xff1c1f25));
    g.drawRoundedRectangle (plot.reduced (0.5f), 5.0f, 1.0f);
}

void SpectrumPanel::mouseUp (const juce::MouseEvent& e)
{
    for (int i = 0; i < tabNames.size(); ++i)
    {
        if (tabRect (i).contains (e.position))
        {
            mode = i;
            repaint();
            return;
        }
    }

    if (e.position.getDistanceFrom (chipRect (0).getCentre()) < 10.0f)
    {
        pre = true;
        repaint();
        return;
    }

    if (e.position.getDistanceFrom (chipRect (1).getCentre()) < 10.0f)
    {
        pre = false;
        repaint();
    }
}

//==============================================================================
Canvas::Canvas (MHSignatureAudioProcessor& p) : proc (p)
{
    setOpaque (true);

    portrait = juce::ImageFileFormat::loadFrom (BinaryData::portrait_jpg,
                                                (size_t) BinaryData::portrait_jpgSize);

    inMeter  = std::make_unique<LevelMeter> (proc, true);
    outMeter = std::make_unique<LevelMeter> (proc, false);
    addAndMakeVisible (*inMeter);
    addAndMakeVisible (*outMeter);

    tabs = std::make_unique<TabStrip>();
    tabs->onTabChanged = [this] (int i) { applyFocus (i); };
    addAndMakeVisible (*tabs);

    presetBar = std::make_unique<PresetBar> (proc);
    addAndMakeVisible (*presetBar);

    spectrumPanel = std::make_unique<SpectrumPanel> (proc);
    addAndMakeVisible (*spectrumPanel);

    // ---------------------------------------------------------- SIGNATURE ENGINE
    enginePanel = std::make_unique<ModulePanel> (proc, "SIGNATURE ENGINE", false);
    enginePanel->setCaption ("TURN IDEAS INTO SOUND");
    enginePanel->customLayout = [this]
    {
        if (enginePanel == nullptr) return;

        const int w = enginePanel->getWidth();
        const int cx = w / 2;
        const int cyTop = 92, cyBot = 192;
        const int cy = (cyTop + cyBot) / 2;

        if (engineKnob != nullptr)
            engineKnob->setBounds (cx - 78, cy - 78, 156, 156);

        const int mx[4] { cx - 166, cx + 166, cx - 166, cx + 166 };
        const int my[4] { cyTop, cyTop, cyBot, cyBot };

        for (int i = 0; i < (int) engineMacros.size() && i < 4; ++i)
            engineMacros[(size_t) i]->setBounds (mx[i] - 30, my[i] - 30, 60, 84);
    };
    addAndMakeVisible (*enginePanel);

    {
        auto addMacro = [this] (const char* id, const char* label)
        {
            auto k = std::make_unique<MhKnob> (proc, id, label, Fmt::int0);
            enginePanel->addAndMakeVisible (*k);
            engineMacros.push_back (std::move (k));
        };

        addMacro (Params::character, "CHARACTER");
        addMacro (Params::density,   "DENSITY");
        addMacro (Params::space,     "SPACE");
        addMacro (Params::width,     "WIDTH");

        engineKnob = std::make_unique<MhKnob> (proc, Params::intensity, "", Fmt::pct0, true, true);
        enginePanel->addAndMakeVisible (*engineKnob);
        enginePanel->resized();
    }

    // ------------------------------------------------------------------------ EQ
    eqPanel = std::make_unique<ModulePanel> (proc, "EQ");
    eqPanel->addPower (Params::eqOn, "EQ");
    eqPanel->addKnob (Params::eqHpf,   "HPF",    Fmt::hz);
    eqPanel->addKnob (Params::eqLowG,  "LOW",    Fmt::db1);
    eqPanel->addKnob (Params::eqLmG,   "LO MID", Fmt::db1);
    eqPanel->addKnob (Params::eqHmG,   "HI MID", Fmt::db1);
    eqPanel->addKnob (Params::eqHighG, "HIGH",   Fmt::db1);
    eqPanel->setReadout ("4 BANDS");
    eqPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::eqCurve (g, a, proc);
    });
    addAndMakeVisible (*eqPanel);

    // ----------------------------------------------------------- DYNAMICS + LIMIT
    dynPanel = std::make_unique<ModulePanel> (proc, "DYNAMICS");
    dynPanel->addPower (Params::dynOn, "COMP");
    dynPanel->addPower (Params::limOn, "LIMIT");
    dynPanel->addKnob (Params::dynThr,     "THRESH",  Fmt::db1);
    dynPanel->addKnob (Params::dynRatio,   "RATIO",   Fmt::ratio);
    dynPanel->addKnob (Params::dynAtt,     "ATTACK",  Fmt::ms);
    dynPanel->addKnob (Params::dynRel,     "RELEASE", Fmt::ms);
    dynPanel->addKnob (Params::dynMakeup,  "MAKEUP",  Fmt::db1);
    dynPanel->addKnob (Params::limCeiling, "CEILING", Fmt::db1);
    dynPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::compCurve (g, a, proc);
    });
    addAndMakeVisible (*dynPanel);


    // --------------------------------------------------------------------- COLOR
    satPanel = std::make_unique<ModulePanel> (proc, "SATURATION");
    satPanel->addPower (Params::satOn, "SAT");
    satPanel->addKnob (Params::satDrive, "DRIVE", Fmt::raw1);
    satPanel->addKnob (Params::satTone,  "TONE",  Fmt::pct0);
    satPanel->addKnob (Params::satMix,   "MIX",   Fmt::pct0);
    satPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::saturationCurve (g, a);
    });
    addAndMakeVisible (*satPanel);

    satType = std::make_unique<ChoiceChip> (proc, Params::satType, "CHARACTER");
    satPanel->addAndMakeVisible (*satType);
    satPanel->customLayout = [this]
    {
        satType->setBounds (satPanel->getWidth() - 137, 7, 124, 18);
    };
    satPanel->resized();

    // --------------------------------------------------------------------- VOCAL
    vocPanel = std::make_unique<ModulePanel> (proc, "VOCAL CONTROL");
    vocPanel->addPower (Params::vocOn, "VOX");
    vocPanel->addKnob (Params::vocHpf,    "HPF",    Fmt::hz);
    vocPanel->addKnob (Params::vocBodyG,  "BODY",   Fmt::db1);
    vocPanel->addKnob (Params::vocDeessF, "DE-ESS", Fmt::hz);
    vocPanel->addKnob (Params::vocPresG,  "PRES",   Fmt::db1);
    vocPanel->addKnob (Params::vocAirG,   "AIR",    Fmt::db1);
    vocPanel->setReadout ("DE-ESS 6.5 k");
    addAndMakeVisible (*vocPanel);

    // -------------------------------------------------------------------- REVERB
    spacePanel = std::make_unique<ModulePanel> (proc, "REVERB");
    spacePanel->addPower (Params::revOn, "REV");
    spacePanel->addKnob (Params::revDecay, "DECAY",  Fmt::sec1);
    spacePanel->addKnob (Params::revPre,   "PREDLY", Fmt::ms);
    spacePanel->addKnob (Params::revDamp,  "DAMP",   Fmt::pct0);
    spacePanel->addKnob (Params::revMix,   "MIX",    Fmt::pct0);
    spacePanel->setReadout ("3.4 s");
    spacePanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::decayCurve (g, a, 2.6f);
    });
    addAndMakeVisible (*spacePanel);

    // --------------------------------------------------------------------- DELAY
    delayPanel = std::make_unique<ModulePanel> (proc, "DELAY");
    delayPanel->addPower (Params::dlyOn, "DLY");
    delayPanel->addKnob (Params::dlyTime,   "TIME",   Fmt::ms);
    delayPanel->addKnob (Params::dlyFb,     "FDBK",   Fmt::pct0);
    delayPanel->addKnob (Params::dlyFilter, "FILTER", Fmt::hz);
    delayPanel->addKnob (Params::dlyMix,    "MIX",    Fmt::pct0);
    delayPanel->setReadout ("375 ms");
    delayPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::delayDots (g, a, proc);
    });
    addAndMakeVisible (*delayPanel);

    // -------------------------------------------------------------------- IMAGER
    imgPanel = std::make_unique<ModulePanel> (proc, "IMAGER");
    imgPanel->addPower (Params::imgOn, "IMG");
    imgPanel->addKnob (Params::imgWidth,   "WIDTH", Fmt::pct0);
    imgPanel->addKnob (Params::imgBalance, "BAL",   Fmt::int0);
    imgPanel->addKnob (Params::imgMonoF,   "MONO",  Fmt::hz);
    imgPanel->setReadout ("M/S 92");
    imgPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::imagerFan (g, a, proc);
    });
    addAndMakeVisible (*imgPanel);

    // ---------------------------------------------------------------- MODULATION
    modPanel = std::make_unique<ModulePanel> (proc, "MODULATION");
    modPanel->addPower (Params::modOn, "MOD");
    modPanel->addKnob (Params::modRate,  "RATE",  Fmt::raw1);
    modPanel->addKnob (Params::modDepth, "DEPTH", Fmt::pct0);
    modPanel->addKnob (Params::modMix,   "MIX",   Fmt::pct0);
    modPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::lfoCurve (g, a);
    });
    addAndMakeVisible (*modPanel);

    // ----------------------------------------------------------------- TRANSIENT
    trnPanel = std::make_unique<ModulePanel> (proc, "TRANSIENT");
    trnPanel->addPower (Params::trnOn, "TRNS");
    trnPanel->addKnob (Params::trnAtt,  "ATTACK",  Fmt::int0);
    trnPanel->addKnob (Params::trnSus,  "SUSTAIN", Fmt::int0);
    trnPanel->addKnob (Params::trnFreq, "SPLIT",   Fmt::hz);
    trnPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::transientCurve (g, a, proc);
    });
    addAndMakeVisible (*trnPanel);

    // -------------------------------------------------------------------- UTILITY
    ioPanel = std::make_unique<ModulePanel> (proc, "UTILITY / I/O", false);
    ioPanel->addKnob (Params::ioInput,  "INPUT",  Fmt::db1);
    ioPanel->addKnob (Params::ioOutput, "OUTPUT", Fmt::db1);
    ioPanel->addKnob (Params::ioMix,    "MIX",    Fmt::pct0);
    ioPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::ioRails (g, a);
    });
    addAndMakeVisible (*ioPanel);

    monoSwitch = std::make_unique<PowerSwitch> (proc, Params::ioMono, "MONO");
    bypassSwitch = std::make_unique<PowerSwitch> (proc, Params::ioBypass, "BYPASS");
    ioPanel->addAndMakeVisible (*monoSwitch);
    ioPanel->addAndMakeVisible (*bypassSwitch);
    ioPanel->customLayout = [this]
    {
        const int w = ioPanel->getWidth();
        monoSwitch->setBounds (w - 152, 7, 46, 18);
        bypassSwitch->setBounds (w - 100, 7, 46, 18);
    };

    ioPanel->setReadout ("");
    ioPanel->resized();

    buildLayout();
    setSize (designW, designH);
    startTimerHz (24);
}

Canvas::~Canvas() = default;

//==============================================================================
void Canvas::buildLayout()
{
    inMeter->setBounds  (geo::inBox);
    outMeter->setBounds (geo::outBox);

    tabs->setBounds      (geo::tabBar);
    presetBar->setBounds (geo::preset);

    enginePanel->setBounds   (geo::engine);
    spectrumPanel->setBounds (geo::spectrum);

    eqPanel->setBounds    (geo::eq);
    dynPanel->setBounds   (geo::dyn);
    satPanel->setBounds   (geo::colour);

    vocPanel->setBounds   (geo::vocal);
    spacePanel->setBounds (geo::reverb);
    delayPanel->setBounds (geo::delay);
    imgPanel->setBounds   (geo::imager);

    modPanel->setBounds   (geo::mod);
    trnPanel->setBounds   (geo::transient);
    ioPanel->setBounds    (geo::io);

    applyFocus (tabs->getIndex());
}

void Canvas::applyFocus (int tab)
{
    auto focus = [] (const std::unique_ptr<ModulePanel>& p, bool on)
    {
        if (p != nullptr) p->setFocused (on);
    };

    focus (eqPanel,    tab == 0 || tab == 1);
    focus (dynPanel,   tab == 0 || tab == 2);
    focus (vocPanel,   tab == 0 || tab == 2);
    focus (trnPanel,   tab == 0 || tab == 2);
    focus (satPanel,   tab == 0 || tab == 3);
    focus (spacePanel, tab == 0 || tab == 4);
    focus (modPanel,   tab == 0 || tab == 5);
    focus (imgPanel,   tab == 0 || tab == 6);
}

void Canvas::timerCallback()
{
    lfoPhase += 0.0065f;
    if (lfoPhase > 1.0f) lfoPhase -= 1.0f;

    engineGlow = juce::jmin (1.0f, 0.15f + proc.getEngineActivity() * 1.3f);
    if (engineKnob != nullptr) engineKnob->setAccentGlow (engineGlow);

    inMeter->repaint();
    outMeter->repaint();

    dynPanel->setReadout ("GR " + juce::String (proc.getGainReductionDb(), 1));
    presetBar->repaint();
}

//==============================================================================
void Canvas::paint (juce::Graphics& g)
{
    g.fillAll (noir);

    // ------------------------------------------------------------- portrait rail
    const auto rail = geo::portrait.toFloat();

    if (portrait.isValid())
    {
        g.drawImage (portrait, rail, juce::RectanglePlacement::stretchToFit, false);
    }
    else
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a1108), rail.getX(), rail.getY(),
                                                 noir, rail.getX(), rail.getBottom(), false));
        g.fillRect (rail);
    }

    {
        // top and bottom melt into the black of the interface
        g.setGradientFill (juce::ColourGradient (noir, rail.getX(), rail.getY(),
                                                 juce::Colours::transparentBlack,
                                                 rail.getX(), rail.getY() + 62.0f, false));
        g.fillRect (rail.withHeight (62.0f));

        const float fadeTop = rail.getY() + rail.getHeight() * 0.70f;
        g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack,
                                                 rail.getX(), fadeTop, noir,
                                                 rail.getX(), rail.getBottom(), false));
        g.fillRect (rail.withTop (fadeTop));

        g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack,
                                                 rail.getRight() - 38.0f, 0.0f, noir,
                                                 rail.getRight(), 0.0f, false));
        g.fillRect (rail.withTrimmedLeft (rail.getWidth() - 38.0f));
    }

    // signature
    {
        const float ccx = rail.getCentreX();

        g.setFont (juce::Font (juce::Font::getDefaultSerifFontName(), 21.0f, juce::Font::italic));
        g.setColour (juce::Colour (0xfff7f1eb));
        g.drawText ("Milad Hashemi", juce::Rectangle<float> (rail.getX(), 624.0f,
                                                             rail.getWidth(), 34.0f),
                    juce::Justification::centred, false);

        juce::Path flourish;
        flourish.startNewSubPath (ccx - 75.0f, 665.0f);
        flourish.cubicTo (ccx - 40.0f, 658.0f, ccx - 10.0f, 661.0f, ccx + 10.0f, 662.0f);
        flourish.cubicTo (ccx + 35.0f, 663.0f, ccx + 52.0f, 666.0f, ccx + 60.0f, 662.0f);
        g.setColour (accent.withAlpha (0.9f));
        g.strokePath (flourish, juce::PathStrokeType (1.2f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

        fitText (g, "MUSIC  \xc2\xb7  PEOPLE  \xc2\xb7  EMOTION  \xc2\xb7  ALWAYS",
                 { rail.getX(), 676.0f, rail.getWidth(), 14.0f },
                 6.6f, juce::Colour (0xffc6beb8), juce::Justification::centred, false, 1.6f);

        fitText (g, "VERSION 1.0.0", { rail.getX(), 806.0f, rail.getWidth(), 14.0f },
                 6.6f, faint, juce::Justification::centred, false, 2.6f);

        g.setColour (juce::Colour (0xff1c1f25));
        g.drawLine (rail.getRight() + 0.5f, rail.getY(), rail.getRight() + 0.5f,
                    rail.getBottom(), 1.0f);
    }

    // a vermilion haze under the engine, so the dial reads as the lamp of the board
    {
        const auto e = geo::engine.toFloat();

        g.setColour (deepAccent.withAlpha (0.10f));
        g.fillRoundedRectangle (e.expanded (16.0f), 22.0f);
        g.setColour (deepAccent.withAlpha (0.08f));
        g.fillRoundedRectangle (e.expanded (7.0f), 14.0f);
    }

    // -------------------------------------------------------------------- header
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0a0b0e), 0.0f, 0.0f,
                                                 juce::Colour (0xff07080a), 0.0f, 65.0f, false));
        g.fillRect (juce::Rectangle<float> (0.0f, 0.0f, (float) designW, 65.0f));

        g.setColour (juce::Colour (0xff1a1d23));
        g.drawLine (0.0f, 65.0f, (float) designW, 65.0f, 1.0f);

        // monogram: two M strokes and the vermilion stem
        {
            juce::Path mono;
            const float x = 24.0f, y = 20.0f, h = 26.0f, w = h * 0.40f;

            for (int k = 0; k < 2; ++k)
            {
                const float ox = x + (float) k * (w + h * 0.16f);
                mono.startNewSubPath (ox, y + h);
                mono.lineTo (ox, y);
                mono.lineTo (ox + w, y + h * 0.60f);
                mono.lineTo (ox + w, y + h);
            }

            g.setColour (ink);
            g.strokePath (mono, juce::PathStrokeType (1.7f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::square));

            g.setColour (accent);
            g.drawLine (x + w + h * 0.08f, y + h * 0.30f, x + w + h * 0.08f, y + h, 1.4f);
        }

        g.setColour (juce::Colour (0xff21252b));
        g.drawLine (84.0f, 12.0f, 84.0f, 54.0f, 1.0f);

        fitText (g, "MH SIGNATURE", { 96.0f, 15.0f, 330.0f, 24.0f }, 13.4f, ink,
                 juce::Justification::centredLeft, false, 4.2f);

        fitText (g, "BY MILAD HASHEMI", { 97.0f, 37.0f, 300.0f, 16.0f }, 7.2f, dim,
                 juce::Justification::centredLeft, false, 3.2f);

        fitText (g, "SOUND WITH", { 1320.0f, 14.0f, 106.0f, 14.0f }, 6.8f, dim,
                 juce::Justification::centredRight, false, 2.6f);
        fitText (g, "INTENTION", { 1320.0f, 28.0f, 106.0f, 14.0f }, 6.8f, dim,
                 juce::Justification::centredRight, false, 2.6f);
    }

    // -------------------------------------------------------------------- footer
    {
        g.setColour (juce::Colour (0xff191c22));
        g.drawLine (0.0f, 864.0f, (float) designW, 864.0f, 1.0f);

        fitText (g, "VERSION 1.0.0", { 310.0f, 870.0f, 200.0f, 14.0f }, 7.2f, faint,
                 juce::Justification::centredLeft, false, 2.6f);

        fitText (g, "AN ALL-IN-ONE MIXING ENVIRONMENT", { 560.0f, 870.0f, 552.0f, 14.0f },
                 8.8f, juce::Colour (0xffacb3bc), juce::Justification::centred, false, 3.0f);

        fitText (g, "DESIGNED BY MILAD HASHEMI", { 1180.0f, 870.0f, 246.0f, 14.0f },
                 7.2f, faint, juce::Justification::centredRight, false, 2.6f);
    }

    // ------------------------------------------------------------------- engine
    {
        const auto r = geo::engine.toFloat();

        fitText (g, "SIGNATURE ENGINE", { r.getX(), r.getY() + 11.0f, r.getWidth(), 20.0f },
                 10.0f, ink, juce::Justification::centred, true, 5.0f);

        g.setColour (juce::Colour (0xff1a1d23));
        g.drawLine (r.getX() + 13.0f, r.getY() + 33.0f, r.getRight() - 13.0f,
                    r.getY() + 33.0f, 1.0f);

        const float cx = r.getCentreX();
        const float cy = r.getY() + 142.0f;
        const float bigR = 74.0f;

        fitText (g, "INTENSITY", { cx - 80.0f, cy + bigR + 5.0f, 160.0f, 14.0f },
                 9.0f, ink2, juce::Justification::centred, true, 3.0f);
        fitText (g, formatValue (pv (proc, Params::intensity), Fmt::pct0),
                 { cx - 80.0f, cy + bigR + 18.0f, 160.0f, 14.0f },
                 8.6f, softAccent, juce::Justification::centred, false, 0.8f);
    }
}

} // namespace mhui

//==============================================================================
MHSignatureAudioProcessorEditor::MHSignatureAudioProcessorEditor (MHSignatureAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), canvas (p)
{
    setOpaque (true);
    addAndMakeVisible (canvas);

    setResizable (true, true);
    setResizeLimits ((int) (mhui::Canvas::designW * 0.85f), (int) (mhui::Canvas::designH * 0.85f),
                     mhui::Canvas::designW * 2, mhui::Canvas::designH * 2);

    if (auto* constrainer = getConstrainer())
        constrainer->setFixedAspectRatio ((double) mhui::Canvas::designW
                                          / (double) mhui::Canvas::designH);

    float scale = 1.0f;
    if (auto* d = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
    {
        const auto area = d->userArea;
        scale = juce::jmin (1.0f, (float) (area.getHeight() - 60) / (float) mhui::Canvas::designH);
        scale = juce::jmin (scale, (float) (area.getWidth() - 40) / (float) mhui::Canvas::designW);
    }

    scale = juce::jlimit (0.85f, 1.0f, scale);

    setSize (juce::roundToInt ((float) mhui::Canvas::designW * scale),
             juce::roundToInt ((float) mhui::Canvas::designH * scale));
}

MHSignatureAudioProcessorEditor::~MHSignatureAudioProcessorEditor() = default;

void MHSignatureAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (mhui::noir);
}

void MHSignatureAudioProcessorEditor::resized()
{
    const float scale = juce::jmin ((float) getWidth() / (float) mhui::Canvas::designW,
                                    (float) getHeight() / (float) mhui::Canvas::designH);

    const float w = (float) mhui::Canvas::designW * scale;
    const float h = (float) mhui::Canvas::designH * scale;

    canvas.setBounds (0, 0, mhui::Canvas::designW, mhui::Canvas::designH);
    canvas.setTransform (juce::AffineTransform::scale (scale)
                             .translated (((float) getWidth() - w) * 0.5f,
                                          ((float) getHeight() - h) * 0.5f));
}
