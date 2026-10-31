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

juce::Font monoFont (float size, float kerning)
{
    return juce::Font (juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain)
             .withExtraKerningFactor (kerning * 0.05f);
}

namespace
{
    float pv (MHSignatureAudioProcessor& p, const char* id)
    {
        if (auto* a = p.apvts.getRawParameterValue (id))
            return a->load();
        return 0.0f;
    }

    bool pb (MHSignatureAudioProcessor& p, const char* id)
    {
        return pv (p, id) >= 0.5f;
    }

    /** the knob scale: 270 degrees, starting bottom-left */
    inline float startAngle() { return juce::MathConstants<float>::pi * -0.75f; }
    inline float endAngle()   { return juce::MathConstants<float>::pi *  0.75f; }
}

//==============================================================================
namespace art
{
    void text (juce::Graphics& g, const juce::String& str, juce::Rectangle<float> area,
               float size, juce::Colour colour, juce::Justification just,
               bool bold, float kerning)
    {
        if (str.isEmpty() || area.getWidth() < 3.0f) return;

        auto font = uiFont (size, bold, kerning);
        const float need = font.getStringWidthFloat (str);

        if (need > area.getWidth() && need > 0.0f)
            font = uiFont (size * juce::jmax (0.55f, area.getWidth() / need), bold, kerning);

        g.setFont (font);
        g.setColour (colour);
        g.drawText (str, area, just, false);
    }

    void plate (juce::Graphics& g, juce::Rectangle<float> area, float radius)
    {
        const auto r = area.reduced (0.5f);

        g.setColour (juce::Colours::black.withAlpha (0.62f));
        g.fillRoundedRectangle (r.translated (1.0f, 3.5f).expanded (2.0f), radius + 2.0f);

        g.setGradientFill (juce::ColourGradient (plateTop, r.getX(), r.getY(),
                                                 plateBot, r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, radius);

        {
            juce::Graphics::ScopedSaveState state (g);
            juce::Path clip;
            clip.addRoundedRectangle (r, radius);
            g.reduceClipRegion (clip);

            // brushed grain
            for (float y = r.getY() + 2.0f; y < r.getBottom() - 1.0f; y += 3.0f)
            {
                const float a = 0.018f + 0.012f * std::sin (y * 1.7f);
                g.setColour (juce::Colours::white.withAlpha (a));
                g.drawLine (r.getX() + 1.0f, y, r.getRight() - 1.0f, y, 1.0f);
            }

            // lamp sheen from the upper left
            g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.035f),
                                                     r.getX(), r.getY(),
                                                     juce::Colours::transparentBlack,
                                                     r.getRight(), r.getBottom(), false));
            g.fillRect (r);
        }

        g.setColour (plateEdge);
        g.drawRoundedRectangle (r, radius, 1.0f);

        g.setColour (bevelHi.withAlpha (0.55f));
        g.drawLine (r.getX() + radius + 3.0f, r.getY() + 1.0f,
                    r.getRight() - radius - 3.0f, r.getY() + 1.0f, 1.0f);
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.drawLine (r.getX() + radius + 3.0f, r.getBottom() - 1.0f,
                    r.getRight() - radius - 3.0f, r.getBottom() - 1.0f, 1.0f);
    }

    void screws (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const juce::Point<float> pts[] = {
            { area.getX() + 11.0f, area.getY() + 11.0f },
            { area.getRight() - 11.0f, area.getY() + 11.0f },
            { area.getX() + 11.0f, area.getBottom() - 11.0f },
            { area.getRight() - 11.0f, area.getBottom() - 11.0f }
        };

        const float angles[] { 0.5f, 2.1f, 1.3f, 0.2f };

        for (int i = 0; i < 4; ++i)
        {
            const auto c = pts[i];
            const float r = 3.0f;

            g.setColour (juce::Colour (0xff3a3e45));
            g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);
            g.setColour (juce::Colour (0xff4a4f57));
            g.fillEllipse (c.x - r * 0.62f, c.y - r * 0.62f, r * 1.24f, r * 1.24f);

            const float a = angles[i];
            g.setColour (juce::Colour (0xff1a1d21));
            g.drawLine (c.x - std::cos (a) * r * 0.55f, c.y - std::sin (a) * r * 0.55f,
                        c.x + std::cos (a) * r * 0.55f, c.y + std::sin (a) * r * 0.55f, 1.0f);
        }
    }

    void ledBox (juce::Graphics& g, juce::Rectangle<float> area, const juce::String& str,
                 float size, juce::Colour colour)
    {
        g.setColour (ledBg);
        g.fillRoundedRectangle (area, 2.5f);
        g.setColour (juce::Colour (0xff282c32));
        g.drawRoundedRectangle (area.reduced (0.5f), 2.5f, 1.0f);
        g.setColour (juce::Colours::black.withAlpha (0.8f));
        g.drawLine (area.getX() + 3.0f, area.getY() + 1.2f,
                    area.getRight() - 3.0f, area.getY() + 1.2f, 1.0f);

        g.setFont (monoFont (size, 0.4f));
        g.setColour (colour);
        g.drawText (str, area.reduced (2.0f, 0.0f), juce::Justification::centred, false);
    }

    void rocker (juce::Graphics& g, juce::Rectangle<float> area, bool on, bool hover)
    {
        const float corner = area.getHeight() * 0.5f;

        // recessed slot
        g.setColour (juce::Colour (0xff0a0b0d));
        g.fillRoundedRectangle (area, corner);
        g.setColour (hover ? juce::Colour (0xff5a616b) : juce::Colour (0xff34383f));
        g.drawRoundedRectangle (area.reduced (0.5f), corner, 1.0f);
        g.setColour (juce::Colours::black);
        g.drawLine (area.getX() + 3.0f, area.getY() + 1.5f,
                    area.getRight() - 3.0f, area.getY() + 1.5f, 1.0f);

        // the lever
        const float lw = area.getWidth() * 0.52f;
        const float lx = on ? area.getRight() - lw - 3.0f : area.getX() + 3.0f;
        const juce::Rectangle<float> lever (lx, area.getY() + 2.4f, lw, area.getHeight() - 4.8f);

        g.setColour (juce::Colour (0xff3b3f46));
        g.fillRoundedRectangle (lever, lever.getHeight() * 0.5f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff5c626b), lever.getX(),
                                                 lever.getY(), juce::Colour (0xff2e3238),
                                                 lever.getX(), lever.getBottom(), false));
        g.fillRoundedRectangle (lever.reduced (0.8f), lever.getHeight() * 0.5f);
    }
}

//==============================================================================
MhKnob::MhKnob (MHSignatureAudioProcessor& proc, const juce::String& paramId,
                const juce::String& labelText, Fmt format, bool ticks, bool hideText)
    : juce::Slider (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox),
      knobLabel (labelText), fmt (format), showTicks (ticks), noText (hideText)
{
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setWantsKeyboardFocus (false);
    setMouseDragSensitivity (240);          // a full sweep needs 240 px: fine control

    if (auto* p = proc.apvts.getParameter (paramId))
    {
        param = p;

        const auto range = p->getNormalisableRange();
        setRange (range.start, range.end, 0.0);
        setSkewFactor (range.skew, range.symmetricSkew);
        setValue (range.convertFrom0to1 (p->getValue()), juce::dontSendNotification);
        setDoubleClickReturnValue (true, range.convertFrom0to1 (p->getDefaultValue()));
        setTooltip (p->getName (48));
    }

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, paramId, *this);

    onValueChange = [this] { repaint(); };
}

void MhKnob::setDimmed (bool shouldDim)
{
    if (dimmed != shouldDim) { dimmed = shouldDim; repaint(); }
}

void MhKnob::mouseEnter (const juce::MouseEvent&) { hovered = true;  repaint(); }
void MhKnob::mouseExit  (const juce::MouseEvent&) { hovered = false; repaint(); }

void MhKnob::mouseDoubleClick (const juce::MouseEvent& e)
{
    juce::Slider::mouseDoubleClick (e);     // the slider resets to its return value
    repaint();
}

void MhKnob::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    if (b.getWidth() < 22.0f || b.getHeight() < 30.0f) return;

    // The block is 2r + 44 high and at least 2r + 26 wide, so this recovers the
    // radius the module laid the dial out with.
    const float r = juce::jlimit (8.0f, 56.0f,
                                  juce::jmin ((b.getWidth() - 26.0f) * 0.5f,
                                              (b.getHeight() - 44.0f) * 0.5f));
    if (r < 7.0f) return;

    const float cx = b.getCentreX();
    const float cy = b.getY() + r;

    const float a0 = startAngle();
    const float a1 = endAngle();
    const float sweep = a1 - a0;
    const double value = getValue();
    const float norm = (float) juce::jlimit (0.0, 1.0, valueToProportionOfLength (value));

    g.setOpacity (dimmed ? 0.42f : 1.0f);

    // ---- printed scale
    if (showTicks && r >= 10.0f)
    {
        const bool big = r >= 26.0f;
        const int divisions = big ? 30 : 20;

        for (int i = 0; i <= divisions; ++i)
        {
            const bool major = (i % (big ? 3 : 2)) == 0;
            if (! big && ! major) continue;

            const float ang = a0 + sweep * (float) i / (float) divisions;
            const float rr0 = r + 3.4f;
            const float rr1 = r + (major ? 7.4f : 5.2f);

            g.setColour (major ? juce::Colour (0xff848a93) : juce::Colour (0xff545a62));
            g.drawLine (cx + std::sin (ang) * rr0, cy - std::cos (ang) * rr0,
                        cx + std::sin (ang) * rr1, cy - std::cos (ang) * rr1,
                        major ? 0.9f : 0.7f);
        }
    }

    // ---- value arc, on exactly the pointer's angle
    if (norm > 0.004f)
    {
        juce::Path arc;
        arc.addCentredArc (cx, cy, r + 1.4f, r + 1.4f, 0.0f, a0, a0 + sweep * norm, true);

        g.setColour (accent.withAlpha (hovered ? 0.22f : 0.13f));
        g.strokePath (arc, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved));
        g.setColour (hovered ? softAccent : accent);
        g.strokePath (arc, juce::PathStrokeType (1.9f, juce::PathStrokeType::curved));
    }

    // ---- knurled skirt
    const juce::Rectangle<float> skirt (cx - r, cy - r, r * 2.0f, r * 2.0f);
    g.setColour (juce::Colour (0xff111216));
    g.fillEllipse (skirt);
    g.setColour (juce::Colour (0xff383d45));
    g.drawEllipse (skirt.reduced (0.5f), 1.0f);

    {
        const int n = r > 26.0f ? 72 : 56;

        for (int i = 0; i < n; ++i)
        {
            const float ang = a0 + sweep * (float) i / (float) (n - 1);
            g.setColour ((i % 2) == 0 ? juce::Colour (0xff40454d) : juce::Colour (0xff1a1c21));
            g.drawLine (cx + std::sin (ang) * (r - 1.2f), cy - std::cos (ang) * (r - 1.2f),
                        cx + std::sin (ang) * (r * 0.80f), cy - std::cos (ang) * (r * 0.80f),
                        0.7f);
        }
    }

    // ---- brushed metal cap
    const float cap = r * 0.72f;
    const juce::Rectangle<float> capArea (cx - cap, cy - cap, cap * 2.0f, cap * 2.0f);

    g.setColour (juce::Colour (0xff0c0d0f));
    g.fillEllipse (capArea.expanded (1.2f));

    g.setGradientFill (juce::ColourGradient (metalTop, cx - cap * 0.35f, cy - cap * 0.45f,
                                             metalBot, cx + cap * 0.45f, cy + cap * 0.65f,
                                             true));
    g.fillEllipse (capArea);

    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.fillEllipse (capArea.getX() + capArea.getWidth() * 0.16f,
                   capArea.getY() + capArea.getHeight() * 0.09f,
                   capArea.getWidth() * 0.46f, capArea.getHeight() * 0.24f);

    g.setColour (juce::Colour (0xff262a30));
    g.drawEllipse (capArea.reduced (0.5f), 0.8f);

    // ---- pointer: dark groove with a lit tip
    {
        const float ang = a0 + sweep * norm;
        const juce::Point<float> tip (cx + std::sin (ang) * cap * 0.92f,
                                      cy - std::cos (ang) * cap * 0.92f);
        const juce::Point<float> inner (cx + std::sin (ang) * cap * 0.18f,
                                        cy - std::cos (ang) * cap * 0.18f);

        g.setColour (juce::Colour (0xff16181c));
        g.drawLine (inner.x, inner.y, tip.x, tip.y, juce::jmax (1.6f, r * 0.085f));
        g.setColour (juce::Colour (0xfffafafc));
        g.drawLine (inner.x + 0.4f, inner.y - 0.4f, tip.x, tip.y,
                    juce::jmax (0.8f, r * 0.045f));
    }

    g.setColour (juce::Colour (0xff1c1f24));
    g.fillEllipse (cx - r * 0.07f, cy - r * 0.07f, r * 0.14f, r * 0.14f);

    if (noText) { g.setOpacity (1.0f); return; }

    // ---- printed name, then the digital value window
    const float labelSize = juce::jmin (7.6f, r * 0.44f);
    const float valueSize = juce::jmin (7.0f, r * 0.40f);

    if (knobLabel.isNotEmpty())
        art::text (g, knobLabel, { b.getX() + 1.0f, cy + r + 13.0f, b.getWidth() - 2.0f, 14.0f },
                   labelSize, ink2, juce::Justification::centred, false, 1.5f);

    const juce::String valueText = formatValue ((float) value, fmt);
    const float capW = r >= 40.0f ? 56.0f : 34.0f;
    const float textW = juce::jlimit (24.0f, juce::jmax (26.0f, b.getWidth()),
                                      juce::jmax (juce::jmin (capW, r * 1.5f + 12.0f),
                                                  (float) valueText.length() * (valueSize * 0.62f) + 10.0f));

    const float boxY = cy + r + 25.0f;

    if (boxY + 16.0f <= b.getBottom() + 1.5f)
        art::ledBox (g, { cx - textW * 0.5f, boxY, textW, 16.0f }, valueText, valueSize);

    g.setOpacity (1.0f);
}

//==============================================================================
PowerSwitch::PowerSwitch (MHSignatureAudioProcessor& proc, const juce::String& paramId,
                          const juce::String& shortName, bool compact)
    : juce::Button (paramId), moduleName (shortName), compactStyle (compact)
{
    if (auto* p = proc.apvts.getParameter (paramId))
    {
        param = p;
        state = p->getValue() >= 0.5f;
    }

    setClickingTogglesState (false);        // we drive the parameter ourselves
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setTooltip (shortName + ": click to take this module in or out of the chain");
}

void PowerSwitch::clicked()
{
    if (param != nullptr)
    {
        const bool next = ! (param->getValue() >= 0.5f);
        param->setValueNotifyingHost (next ? 1.0f : 0.0f);
        state = next;
    }
    else
    {
        state = ! state;
    }

    setToggleState (state, juce::dontSendNotification);
    repaint();

    if (onToggle)
        onToggle();
}

void PowerSwitch::refresh()
{
    if (param == nullptr) return;

    const bool now = param->getValue() >= 0.5f;
    if (now != state)
    {
        state = now;
        setToggleState (state, juce::dontSendNotification);
        repaint();
    }
}

void PowerSwitch::paintButton (juce::Graphics& g, bool over, bool)
{
    auto area = getLocalBounds().toFloat();

    juce::Rectangle<float> slot = area;
    juce::Rectangle<float> lamp;
    const bool tight = compactStyle || area.getWidth() < 60.0f;

    if (tight)
    {
        const float sh = juce::jmin (area.getHeight(), 18.0f);
        const float sw = juce::jmin (area.getWidth() - 14.0f, 46.0f);

        slot = { area.getX(), area.getCentreY() - sh * 0.5f, sw, sh };
        lamp = { slot.getRight() + 6.0f, area.getCentreY() - 3.0f, 6.0f, 6.0f };
    }
    else
    {
        slot = area.withWidth (juce::jmin (area.getWidth(), 44.0f));
        lamp = area.withTrimmedLeft (slot.getWidth() + 6.0f).withWidth (8.0f)
                   .withHeight (8.0f).withCentre ({ slot.getRight() + 10.0f,
                                                    area.getCentreY() });
    }

    art::rocker (g, slot, state, over);

    const auto lc = lamp.getCentre();
    const float lr = tight ? 3.0f : 3.4f;

    if (state)
    {
        g.setColour (deepAccent.withAlpha (0.75f));
        g.fillEllipse (lc.x - lr - 2.2f, lc.y - lr - 2.2f, (lr + 2.2f) * 2.0f,
                       (lr + 2.2f) * 2.0f);
        g.setColour (accent);
        g.fillEllipse (lc.x - lr, lc.y - lr, lr * 2.0f, lr * 2.0f);
        g.setColour (softAccent);
        g.fillEllipse (lc.x - lr * 0.4f, lc.y - lr * 0.65f, lr * 0.7f, lr * 0.55f);
    }
    else
    {
        g.setColour (juce::Colour (0xff121317));
        g.fillEllipse (lc.x - lr, lc.y - lr, lr * 2.0f, lr * 2.0f);
        g.setColour (juce::Colour (0xff4a5058));
        g.drawEllipse (lc.x - lr, lc.y - lr, lr * 2.0f, lr * 2.0f, 1.0f);
    }

    if (! tight)
    {
        art::text (g, state ? "ON" : "OFF",
                   area.withTrimmedLeft (lamp.getRight() + 3.0f).withWidth (30.0f),
                   6.6f, state ? softAccent : dimText, juce::Justification::centredLeft,
                   false, 1.2f);
    }
}

//==============================================================================
ModulePanel::ModulePanel (MHSignatureAudioProcessor& p, const juce::String& t)
    : title (t), proc (p)
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

void ModulePanel::addKnob (const juce::String& id, const juce::String& label, Fmt fmt)
{
    auto k = std::make_unique<MhKnob> (proc, id, label, fmt);
    addAndMakeVisible (*k);
    knobs.push_back (std::move (k));
    resized();
}

void ModulePanel::setArt (std::function<void (juce::Graphics&, juce::Rectangle<float>)> drawer)
{
    artDrawer = std::move (drawer);
    repaint();
}

void ModulePanel::setReadout (juce::String t) { readout = std::move (t); repaint(); }

void ModulePanel::setChip (juce::String name, juce::String value)
{
    chipName = std::move (name);
    chipValue = std::move (value);
    repaint();
}

void ModulePanel::setRows (int r)        { rowsWanted = juce::jmax (1, r); resized(); repaint(); }
void ModulePanel::setMaxRadius (float r) { maxR = juce::jmax (8.0f, r); resized(); repaint(); }
void ModulePanel::setArtHeight (float h) { artHeight = juce::jmax (0.0f, h); resized(); repaint(); }
void ModulePanel::setChipRow (bool reserve) { chipRow = reserve; resized(); repaint(); }

void ModulePanel::setSections (juce::StringArray labels)
{
    sections = std::move (labels);
    rowsWanted = juce::jmax (1, sections.size());
    resized();
    repaint();
}

void ModulePanel::refreshPowers()
{
    bool changed = false;

    for (auto& s2 : powers)
    {
        const auto before = s2->getToggleState();
        s2->refresh();
        changed = changed || (before != s2->getToggleState());
    }

    if (changed) repaint();
}

bool ModulePanel::isAnyModuleOn() const
{
    if (powers.empty()) return true;
    for (const auto& s : powers)
        if (s->getToggleState()) return true;
    return false;
}

namespace
{
    /** the top of the dial block, after the chip row and the artwork band */
    inline int dialTop (float chipRowHeight, float art)
    {
        int top = 46;
        if (chipRowHeight > 0.0f) top += (int) chipRowHeight;
        if (art > 0.0f)           top += (int) art + 12;
        return top;
    }
}

juce::Rectangle<int> ModulePanel::dialBox() const
{
    const int top = dialTop (chipRow ? 26.0f : 0.0f, artHeight);
    return { 16, top, juce::jmax (0, getWidth() - 32),
             juce::jmax (0, getHeight() - 8 - top) };
}

juce::Rectangle<int> ModulePanel::artBand() const
{
    const int top = 46 + (chipRow ? 26 : 0);

    if (artHeight < 4.0f) return {};

    return { 26, top, juce::jmax (0, getWidth() - 52),
             juce::jmin ((int) artHeight, juce::jmax (0, getHeight() - 8 - top)) };
}

void ModulePanel::resized()
{
    // ---- the rockers, right to left
    const int n = (int) powers.size();
    int px = getWidth() - 30;

    for (int i = n - 1; i >= 0; --i)
    {
        const bool single = n == 1;
        const int w = single ? 84 : 44;
        px -= w;
        powers[(size_t) i]->setBounds (px, single ? 10 : 11, w, single ? 18 : 16);
        px -= single ? 0 : 8;
    }

    if (customLayout) customLayout();

    if (knobs.empty()) return;

    // ---- two half-height modules in one plate (MOTION)
    if (sections.size() > 0)
    {
        const int secs = sections.size();
        const int top = 46;
        const int rowH = juce::jmax (1, (getHeight() - 8 - top) / secs);
        const int perSec = ((int) knobs.size() + secs - 1) / secs;

        for (int i = 0; i < (int) knobs.size(); ++i)
        {
            const int sec = i / perSec;
            const int ci = i % perSec;
            const int count = juce::jmin (perSec, (int) knobs.size() - sec * perSec);

            const int slotW = juce::jmax (1, (getWidth() - 32) / juce::jmax (1, count));
            const int boxH = juce::jmax (1, rowH - 18);

            const float r = juce::jlimit (8.0f, 56.0f,
                                          juce::jmin ((float) (slotW - 26) * 0.5f,
                                                      (float) (boxH - 56) * 0.5f, maxR));
            const int rr = (int) std::round (r);
            const int blockH = 2 * rr + 44;

            knobs[(size_t) i]->setBounds (16 + ci * slotW,
                                          top + rowH * sec + 16 + (boxH - blockH) / 2,
                                          slotW, blockH);
        }

        return;
    }

    const auto box = dialBox();
    if (box.getHeight() < 30 || box.getWidth() < 40) return;

    const int count = (int) knobs.size();
    const int rows = juce::jmax (1, rowsWanted);
    const int perRow = (count + rows - 1) / rows;
    const int slotW = box.getWidth() / juce::jmax (1, perRow);
    const int slotH = box.getHeight() / rows;

    const float r = juce::jlimit (8.0f, 56.0f,
                                  juce::jmin ((float) (slotW - 26) * 0.5f,
                                              (float) (slotH - 56) * 0.5f, maxR));
    const int rr = (int) std::round (r);
    const int blockH = 2 * rr + 44;

    for (int i = 0; i < count; ++i)
    {
        const int ri = i / perRow, ci = i % perRow;
        knobs[(size_t) i]->setBounds (box.getX() + ci * slotW,
                                      box.getY() + ri * slotH + (slotH - blockH) / 2,
                                      slotW, blockH);
    }
}

void ModulePanel::paint (juce::Graphics& g)
{
    const bool live = isAnyModuleOn();
    const auto r = getLocalBounds().toFloat();

    art::plate (g, r);
    art::screws (g, r);

    art::text (g, title.toUpperCase(), { 30.0f, 9.0f, (float) getWidth() - 40.0f, 22.0f },
               9.4f, live ? ink : muted, juce::Justification::centredLeft, false, 2.4f);

    if (readout.isNotEmpty())
    {
        const float right = (float) getWidth() - 30.0f
                              - (powers.empty() ? 0.0f : (powers.size() == 1 ? 90.0f : 100.0f));
        art::text (g, readout, { right - 100.0f, 9.0f, 100.0f, 22.0f }, 6.8f, dimText,
                   juce::Justification::centredRight, false, 1.2f);
    }

    g.setColour (juce::Colour (0xff0a0b0d));
    g.drawLine (26.0f, 36.0f, (float) getWidth() - 26.0f, 36.0f, 1.0f);
    g.setColour (juce::Colour (0xff2c3036));
    g.drawLine (26.0f, 37.0f, (float) getWidth() - 26.0f, 37.0f, 0.8f);

    // a chip drawn by the panel itself (the COLOUR card uses a live ChoiceChip instead)
    if (chipName.isNotEmpty() && ! chipRow)
    {
        const juce::Rectangle<float> chip (26.0f, 46.0f,
                                           (float) juce::jmin (getWidth() - 52, 132), 18.0f);
        art::ledBox (g, chip, chipValue, 6.6f);
        art::text (g, chipName, chip.withTrimmedRight (10.0f), 5.8f, dimText,
                   juce::Justification::centredLeft, false, 1.4f);
    }

    // ---- section captions (the MOTION card is two half-height modules)
    if (sections.size() > 0)
    {
        const int top = 46;
        const int rowH = juce::jmax (1, (getHeight() - 8 - top) / juce::jmax (1, sections.size()));

        for (int i = 0; i < sections.size(); ++i)
        {
            art::text (g, sections[i], { 30.0f, (float) (top + rowH * i + 1.0f), 200.0f, 14.0f },
                       6.2f, dimText, juce::Justification::centredLeft, false, 2.2f);

            if (i > 0)
            {
                g.setColour (juce::Colour (0xff0c0d10));
                g.drawLine (26.0f, (float) (top + rowH * i - 2), (float) getWidth() - 26.0f,
                            (float) (top + rowH * i - 2), 1.0f);
            }
        }
    }

    if (artDrawer != nullptr)
    {
        auto a = artBand();
        if (a.getHeight() > 10)
            artDrawer (g, a.toFloat());
    }

    if (! live)
    {
        g.setColour (noir.withAlpha (0.55f));
        g.fillRoundedRectangle (r.reduced (0.5f), 6.0f);
    }
}

//==============================================================================
ChoiceChip::ChoiceChip (MHSignatureAudioProcessor& p, const juce::String& paramId,
                        const juce::String& cap)
    : proc (p), caption (cap)
{
    param = proc.apvts.getParameter (paramId);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setTooltip ("Click to step through the saturation characters");
}

void ChoiceChip::paint (juce::Graphics& g)
{
    if (param == nullptr) return;

    auto r = getLocalBounds().toFloat();

    g.setColour (ledBg);
    g.fillRoundedRectangle (r.reduced (0.5f), 2.5f);
    g.setColour (juce::Colour (0xff2c3036));
    g.drawRoundedRectangle (r.reduced (0.5f), 2.5f, 0.9f);
    g.setColour (juce::Colours::black);
    g.drawLine (r.getX() + 3.0f, r.getY() + 1.2f, r.getRight() - 3.0f, r.getY() + 1.2f, 1.0f);

    art::text (g, caption, r.reduced (7.0f, 0.0f).withTrimmedRight (44.0f), 6.0f, dimText,
               juce::Justification::centredLeft, false, 1.5f);

    g.setFont (monoFont (6.6f, 0.8f));
    g.setColour (softAccent);
    g.drawText (param->getCurrentValueAsText().toUpperCase(),
                r.reduced (7.0f, 0.0f), juce::Justification::centredRight, false);
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
AnalyserPanel::AnalyserPanel (MHSignatureAudioProcessor& p) : proc (p)
{
    for (auto& v : hist) v = -90.0f;
    startTimerHz (24);
}

AnalyserPanel::~AnalyserPanel() = default;

void AnalyserPanel::setStatsVisible (bool shouldShow) { stats = shouldShow; repaint(); }

juce::Rectangle<float> AnalyserPanel::tabRect (int i) const
{
    return { 116.0f + 62.0f * (float) i, 8.0f, 58.0f, 18.0f };
}

juce::Rectangle<float> AnalyserPanel::chipRect (int i) const
{
    const float w = (float) getWidth();
    return i == 0 ? juce::Rectangle<float> (w - 62.0f, 13.0f, 7.0f, 7.0f)
                  : juce::Rectangle<float> (w - 30.0f, 13.0f, 7.0f, 7.0f);
}

juce::Rectangle<float> AnalyserPanel::plotArea() const
{
    auto b = getLocalBounds().toFloat().reduced (24.0f, 0.0f);
    b.removeFromTop (34.0f);
    b.removeFromBottom (stats ? 54.0f : 14.0f);
    return b;
}

void AnalyserPanel::timerCallback()
{
    proc.getSpectrum (spec.data(), false);
    proc.getSpectrum (specPre.data(), true);
    proc.getWaveform (wave.data());

    loudAccum = juce::jmax (loudAccum * 0.93f, proc.getOutputLevelDb());
    peakHold = juce::jmax (peakHold - 0.35f, proc.getOutputLevelDb());

    if (++loudCounter >= 2)
    {
        loudCounter = 0;
        hist[(size_t) histIndex] = loudAccum;
        histIndex = (histIndex + 1) % (int) hist.size();
    }

    repaint();
}

void AnalyserPanel::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat();
    art::plate (g, r);
    art::screws (g, r);

    art::text (g, "ANALYSER", { 30.0f, 6.0f, 100.0f, 22.0f }, 9.0f, ink,
               juce::Justification::centredLeft, false, 2.8f);

    for (int i = 0; i < tabNames.size(); ++i)
    {
        auto tb = tabRect (i);
        art::text (g, tabNames[i], tb, 6.2f, i == mode ? ink2 : dimText,
                   juce::Justification::centredLeft, false, 1.5f);

        if (i == mode)
        {
            g.setColour (accent);
            g.fillRoundedRectangle (tb.getX() - 2.0f, tb.getBottom() - 6.0f,
                                    tb.getWidth() - 14.0f, 1.4f, 0.7f);
        }
    }

    // pre / post — only where the plate is wide enough to hold them
    if (getWidth() >= 340.0f)
    {
        auto preChip = chipRect (0);
        if (pre) { g.setColour (deepAccent.withAlpha (0.8f));
                   g.fillEllipse (preChip.expanded (2.0f)); }
        g.setColour (pre ? accent : juce::Colour (0xff171a1e));
        g.fillEllipse (preChip);
        g.setColour (pre ? accent : juce::Colour (0xff4a5058));
        g.drawEllipse (preChip, 1.0f);
        art::text (g, "PRE", { preChip.getRight() + 4.0f, 9.0f, 26.0f, 14.0f }, 6.0f,
                   pre ? ink2 : dimText, juce::Justification::centredLeft, false, 1.4f);

        auto postChip = chipRect (1);
        if (! pre) { g.setColour (deepAccent.withAlpha (0.8f));
                     g.fillEllipse (postChip.expanded (2.0f)); }
        g.setColour (! pre ? accent : juce::Colour (0xff171a1e));
        g.fillEllipse (postChip);
        g.setColour (! pre ? accent : juce::Colour (0xff4a5058));
        g.drawEllipse (postChip, 1.0f);
        art::text (g, "POST", { postChip.getRight() + 4.0f, 9.0f, 30.0f, 14.0f }, 6.0f,
                   ! pre ? ink2 : dimText, juce::Justification::centredLeft, false, 1.4f);
    }

    g.setColour (juce::Colour (0xff0a0b0d));
    g.drawLine (26.0f, 34.0f, r.getRight() - 26.0f, 34.0f, 1.0f);

    auto plot = plotArea();
    g.setColour (juce::Colour (0xff08090b));
    g.fillRoundedRectangle (plot, 3.0f);
    g.setColour (juce::Colour (0xff262a30));
    g.drawRoundedRectangle (plot.reduced (0.5f), 3.0f, 1.0f);

    auto inner = plot.reduced (1.0f).withTrimmedLeft (24.0f).withTrimmedRight (8.0f)
                     .withTrimmedTop (8.0f).withTrimmedBottom (16.0f);

    if (inner.getHeight() < 8.0f) return;

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

    for (int db = -12; db >= -60; db -= 12)
    {
        const float y = dbToY ((float) db);
        g.setColour (juce::Colour (0xff1a1d22));
        g.drawLine (inner.getX(), y, inner.getRight(), y, 1.0f);
        art::text (g, juce::String (db), { plot.getX() + 3.0f, y - 6.0f, 19.0f, 12.0f },
                   5.6f, juce::Colour (0xff4a5058), juce::Justification::centredRight,
                   false, 0.2f);
    }

    const float freqs[]  { 20.0f, 100.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f, 20000.0f };
    const char* labels[] { "20", "100", "500", "1k", "5k", "10k", "20k" };

    for (int i = 0; i < 7; ++i)
    {
        const float x = freqToX (freqs[i]);
        g.setColour (juce::Colour (0xff1a1d22));
        g.drawLine (x, inner.getY(), x, inner.getBottom(), 1.0f);
        art::text (g, labels[i], { x - 16.0f, plot.getBottom() - 14.0f, 32.0f, 12.0f },
                   5.6f, juce::Colour (0xff4a5058), juce::Justification::centred,
                   false, 0.2f);
    }

    if (mode == 0)
    {
        const auto& data = pre ? specPre : spec;
        const int n = (int) data.size();

        juce::Path line, fill;
        for (int i = 0; i < n; ++i)
        {
            const float f = 20.0f * std::pow (1000.0f, ((float) i + 0.5f) / (float) n);
            const float x = freqToX (f);
            const float y = dbToY (data[(size_t) i]);

            if (i == 0) { line.startNewSubPath (x, y); fill.startNewSubPath (x, inner.getBottom()); }
            line.lineTo (x, y);
            fill.lineTo (x, y);
        }
        fill.lineTo (inner.getRight(), inner.getBottom());
        fill.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.22f), 0.0f, inner.getY(),
                                                 accent.withAlpha (0.01f), 0.0f,
                                                 inner.getBottom(), false));
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

    if (! stats) return;

    const int boxes = 4;
    const float bw = (r.getWidth() - 56.0f) / boxes;

    for (int i = 0; i < boxes; ++i)
    {
        const float x = 28.0f + bw * i;
        const char* cap = i == 0 ? "LEVEL" : (i == 1 ? "PEAK" : (i == 2 ? "GAIN RED." : "WIDTH"));
        const float val = i == 0 ? proc.getOutputLevelDb()
                                : (i == 1 ? peakHold
                                          : (i == 2 ? proc.getGainReductionDb()
                                                    : pv (proc, Params::imgWidth)));
        juce::String txt;
        if (i == 3) txt = juce::String ((int) std::round (val)) + "%";
        else        txt = val <= -60.0f ? "-inf" : juce::String (val, 1);

        art::text (g, cap, { x, r.getBottom() - 32.0f, 70.0f, 10.0f }, 5.8f, dimText,
                   juce::Justification::centredLeft, false, 1.6f);
        art::ledBox (g, { x + 38.0f, r.getBottom() - 38.0f, juce::jmin (bw - 44.0f, 62.0f), 15.0f },
                     txt, 7.4f);
    }
}

void AnalyserPanel::mouseUp (const juce::MouseEvent& e)
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

    if (getWidth() < 340.0f) return;

    if (e.position.getDistanceFrom (chipRect (0).getCentre()) < 10.0f)
    {
        pre = true;
        repaint();
    }
    else if (e.position.getDistanceFrom (chipRect (1).getCentre()) < 10.0f)
    {
        pre = false;
        repaint();
    }
}

//==============================================================================
ChainRail::ChainRail (MHSignatureAudioProcessor& p) : proc (p)
{
    inKnob  = std::make_unique<MhKnob> (proc, Params::ioInput,  "IN",  Fmt::db1, false);
    outKnob = std::make_unique<MhKnob> (proc, Params::ioOutput, "OUT", Fmt::db1, false);

    addAndMakeVisible (*inKnob);
    addAndMakeVisible (*outKnob);

    monoSwitch   = std::make_unique<PowerSwitch> (proc, Params::ioMono,   "MONO", true);
    bypassSwitch = std::make_unique<PowerSwitch> (proc, Params::ioBypass, "BYPASS", true);

    monoSwitch->setTooltip ("Mono: folds the output to a single channel");
    bypassSwitch->setTooltip ("Bypass: takes the whole chain out");

    addAndMakeVisible (*monoSwitch);
    addAndMakeVisible (*bypassSwitch);
}

ChainRail::~ChainRail() = default;

void ChainRail::setActive (const juce::String& name) { active = name; repaint(); }

void ChainRail::refresh()
{
    monoSwitch->refresh();
    bypassSwitch->refresh();
    repaint();
}

void ChainRail::resized()
{
    const int w = getWidth();
    const int uy = 682;                      // the utility block, in design units

    inKnob->setBounds  (18, uy + 19, 56, 74);   // dial centre (46, uy + 34), r = 15
    outKnob->setBounds (72, uy + 19, 56, 74);   // dial centre (100, uy + 34)

    monoSwitch->setBounds   (138, uy + 24, 52, 18);
    bypassSwitch->setBounds (138, uy + 48, 52, 18);

    juce::ignoreUnused (w);
}

void ChainRail::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat();
    art::plate (g, r);
    art::screws (g, r);

    art::text (g, "SIGNAL CHAIN", { 20.0f, 14.0f, 140.0f, 16.0f }, 8.0f, ink2,
               juce::Justification::centredLeft, false, 2.8f);

    g.setColour (juce::Colour (0xff0a0b0d));
    g.drawLine (18.0f, 34.0f, r.getRight() - 18.0f, 34.0f, 1.0f);

    struct Row { const char* name; const char* id; };

    const Row rows[] = {
        { "ENGINE",     nullptr },
        { "EQ",         Params::eqOn },
        { "DYNAMICS",   Params::dynOn },
        { "LIMITER",    Params::limOn },
        { "COLOUR",     Params::satOn },
        { "VOCAL",      Params::vocOn },
        { "REVERB",     Params::revOn },
        { "DELAY",      Params::dlyOn },
        { "MODULATION", Params::modOn },
        { "TRANSIENT",  Params::trnOn },
        { "IMAGER",     Params::imgOn },
        { "OUTPUT",     nullptr }
    };

    const juce::String activeList = active;

    float y = 50.0f;

    for (int i = 0; i < (int) juce::numElementsInArray (rows); ++i)
    {
        const auto& row = rows[i];
        const bool isOutput = juce::String (row.name) == "OUTPUT";
        const bool on = isOutput || row.id == nullptr || pb (proc, row.id);
        const bool hot = activeList.contains (juce::String (row.name));

        if (i % 2 == 0)
            g.setColour (hot ? juce::Colour (0xff1c1614) : juce::Colour (0xff14161a));
        if (i % 2 == 0)
            g.fillRoundedRectangle (14.0f, y - 9.0f, r.getWidth() - 28.0f, 18.0f, 2.0f);

        if (hot)
        {
            g.setColour (accent);
            g.fillRect (14.0f, y - 9.0f, 1.6f, 18.0f);
        }

        const float lx = 24.0f;

        if (on)
        {
            g.setColour ((hot ? accent : juce::Colour (0xff962c1a)).withAlpha (0.30f));
            g.fillEllipse (lx - 5.6f, y - 5.6f, 11.2f, 11.2f);
            g.setColour (hot ? accent : juce::Colour (0xff962c1a));
            g.fillEllipse (lx - 3.6f, y - 3.6f, 7.2f, 7.2f);
            g.setColour (hot ? softAccent : juce::Colour (0xffc4604c));
            g.fillEllipse (lx - 1.5f, y - 1.5f, 3.0f, 3.0f);
        }
        else
        {
            g.setColour (juce::Colour (0xff121317));
            g.fillEllipse (lx - 3.2f, y - 3.2f, 6.4f, 6.4f);
            g.setColour (juce::Colour (0xff3e434b));
            g.drawEllipse (lx - 3.2f, y - 3.2f, 6.4f, 6.4f, 0.9f);
        }

        art::text (g, row.name, { 38.0f, y - 8.0f, 120.0f, 16.0f }, 6.8f,
                   hot ? ink : (on ? ink2 : dimText), juce::Justification::centredLeft,
                   false, 1.6f);

        juce::String value = on ? "ON" : "OFF";

        if (row.id == nullptr && ! isOutput)
        {
            value = juce::String (juce::roundToInt (pv (proc, Params::intensity))) + "%";
        }
        else if (isOutput)
        {
            const float db = pv (proc, Params::ioOutput);
            value = (db <= -59.5f ? juce::String ("-inf")
                                  : juce::String (db, 1)) + " dB";
        }

        {
            g.setFont (monoFont (6.4f, 0.6f));
            g.setColour (hot ? softAccent : dimText);
            g.drawText (value, juce::Rectangle<float> (r.getRight() - 92.0f, y - 8.0f, 70.0f,
                                                      16.0f),
                        juce::Justification::centredRight, false);
        }

        y += 26.0f;
    }

    // ------------------------------------------------------------- utility
    const float uy = 682.0f;

    g.setColour (juce::Colour (0xff0a0b0d));
    g.drawLine (18.0f, uy - 12.0f, r.getRight() - 18.0f, uy - 12.0f, 1.0f);

    art::text (g, "UTILITY / I/O", { 20.0f, uy - 6.0f, 140.0f, 16.0f }, 7.0f, ink2,
               juce::Justification::centredLeft, false, 2.2f);

    art::text (g, "MONO", { 196.0f, uy + 25.0f, 60.0f, 14.0f }, 6.0f, dimText,
               juce::Justification::centredLeft, false, 1.4f);
    art::text (g, "BYPASS", { 196.0f, uy + 49.0f, 60.0f, 14.0f }, 6.0f, dimText,
               juce::Justification::centredLeft, false, 1.4f);
}

//==============================================================================
TabStrip::TabStrip()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

juce::Rectangle<float> TabStrip::tabBounds (int i) const
{
    const float widths[] { 46.0f, 40.0f, 44.0f, 58.0f, 62.0f, 44.0f, 62.0f };
    const float gap = 22.0f;

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
        const bool on = i == current;

        art::text (g, items[i], { tb.getX(), 10.0f, tb.getWidth(), 18.0f }, 8.6f,
                   on ? ink : (i == hovered ? ink2 : muted),
                   juce::Justification::centred, false, 2.2f);

        if (on)
        {
            g.setColour (accent);
            g.fillRoundedRectangle (tb.getX() + 2.0f, 33.0f, tb.getWidth() - 4.0f, 1.8f, 0.9f);
            g.setColour (softAccent);
            g.fillEllipse (tb.getCentreX() - 1.6f, 37.5f, 3.2f, 3.2f);
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
    setTooltip ("Click to browse the 100 factory presets — click the arrows to step");
}

void PresetBar::mouseMove (const juce::MouseEvent& e)
{
    const int z = e.position.x < 22.0f ? -1
                                       : (e.position.x > (float) getWidth() - 22.0f ? 1 : 0);
    if (z != hoverZone) { hoverZone = z; repaint(); }
}

void PresetBar::mouseExit (const juce::MouseEvent&) { hoverZone = 0; repaint(); }

void PresetBar::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();

    g.setColour (ledBg);
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (juce::Colour (0xff2e3239));
    g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);

    art::text (g, "PRESET", { r.getX() + 10.0f, r.getY() + 4.0f, 60.0f, 10.0f }, 5.8f,
               dimText, juce::Justification::centredLeft, false, 2.0f);

    juce::String name = proc.getCurrentPresetName();
    if (proc.isPresetEdited()) name += " *";

    g.setFont (monoFont (8.2f, 0.8f));
    g.setColour (hoverZone < 0 ? softAccent : softAccent.withAlpha (0.92f));
    g.drawText (name.toUpperCase(),
                juce::Rectangle<float> (r.getX() + 10.0f, r.getY() + 15.0f,
                                        r.getWidth() - 48.0f, 20.0f),
                juce::Justification::centredLeft, false);

    // step arrows
    g.setColour (hoverZone == -1 ? ink2 : dimText);
    g.drawText ("<", juce::Rectangle<float> (r.getX() + 3.0f, r.getY(), 14.0f, r.getHeight()),
                juce::Justification::centred, false);
    g.setColour (hoverZone == 1 ? ink2 : dimText);
    g.drawText (">", juce::Rectangle<float> (r.getRight() - 16.0f, r.getY(), 14.0f,
                                            r.getHeight()),
                juce::Justification::centred, false);
}

void PresetBar::refresh()
{
    const auto now = proc.getCurrentPresetName() + (proc.isPresetEdited() ? " *" : "");

    if (now != lastShown)
    {
        lastShown = now;
        repaint();
    }
}

void PresetBar::mouseUp (const juce::MouseEvent& e)
{
    if (e.position.x < 22.0f)                          { proc.prevPreset(); repaint(); }
    else if (e.position.x > (float) getWidth() - 22.0f) { proc.nextPreset(); repaint(); }
    else                                               { showBrowser(); }
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
namespace modart
{
    void eq (juce::Graphics& g, juce::Rectangle<float> a, MHSignatureAudioProcessor& proc)
    {
        const float lowG  = pv (proc, Params::eqLowG);
        const float lmG   = pv (proc, Params::eqLmG);
        const float hmG   = pv (proc, Params::eqHmG);
        const float highG = pv (proc, Params::eqHighG);
        const float tilt  = pv (proc, Params::eqTilt);
        const float hpf   = pv (proc, Params::eqHpf);

        g.setColour (juce::Colour (0xff090a0c));
        g.fillRoundedRectangle (a, 3.0f);
        g.setColour (juce::Colour (0xff262a30));
        g.drawRoundedRectangle (a.reduced (0.5f), 3.0f, 1.0f);

        auto inner = a.reduced (6.0f, 6.0f);
        const float mid = inner.getCentreY();

        g.setColour (juce::Colour (0xff181b20));
        for (int i = 1; i < 8; ++i)
            g.drawLine (inner.getX() + inner.getWidth() * (float) i / 8.0f, inner.getY(),
                        inner.getX() + inner.getWidth() * (float) i / 8.0f, inner.getBottom(), 1.0f);
        for (int i = 1; i < 4; ++i)
            g.drawLine (inner.getX(), inner.getY() + inner.getHeight() * (float) i / 4.0f,
                        inner.getRight(), inner.getY() + inner.getHeight() * (float) i / 4.0f, 1.0f);
        g.setColour (juce::Colour (0xff2b3037));
        g.drawLine (inner.getX(), mid, inner.getRight(), mid, 1.0f);

        auto bell = [] (float f, float fc, float oct)
        {
            const float d = std::log2 (juce::jmax (1.0f, f) / fc) / oct;
            return std::exp (-0.5f * d * d);
        };

        juce::Path p;
        juce::Point<float> nodes[4];
        int node = 0;

        for (int i = 0; i < 140; ++i)
        {
            const float t = (float) i / 139.0f;
            const float f = 20.0f * std::pow (1000.0f, t);

            float db = lowG * bell (f, 65.0f, 1.15f)
                     + lmG  * bell (f, 152.0f, 0.85f)
                     + hmG  * bell (f, 3500.0f, 0.95f)
                     + highG * bell (f, 9000.0f, 1.5f)
                     + tilt * (t - 0.5f) * 6.0f;

            if (f < hpf)
                db -= 10.0f * std::log2 (hpf / juce::jmax (20.0f, f));

            const float x = inner.getX() + t * inner.getWidth();
            const float y = mid - (db / 14.0f) * (inner.getHeight() * 0.44f);

            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);

            if (node < 4 && (i == 12 || i == 34 || i == 74 || i == 108))
                nodes[node++] = { x, y };
        }

        g.setColour (softAccent);
        g.strokePath (p, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

        for (int i = 0; i < node; ++i)
        {
            g.setColour (juce::Colour (0xfff6f0ec));
            g.fillEllipse (nodes[i].x - 3.2f, nodes[i].y - 3.2f, 6.4f, 6.4f);
            g.setColour (accent);
            g.drawEllipse (nodes[i].x - 3.2f, nodes[i].y - 3.2f, 6.4f, 6.4f, 1.0f);
        }
    }

    void comp (juce::Graphics& g, juce::Rectangle<float> a, MHSignatureAudioProcessor& proc)
    {
        g.setColour (juce::Colour (0xff090a0c));
        g.fillRoundedRectangle (a, 3.0f);
        g.setColour (juce::Colour (0xff262a30));
        g.drawRoundedRectangle (a.reduced (0.5f), 3.0f, 1.0f);

        auto inner = a.reduced (6.0f, 6.0f);

        g.setColour (juce::Colour (0xff181b20));
        for (int i = 1; i < 5; ++i)
        {
            g.drawLine (inner.getX() + inner.getWidth() * (float) i / 5.0f, inner.getY(),
                        inner.getX() + inner.getWidth() * (float) i / 5.0f, inner.getBottom(), 1.0f);
            g.drawLine (inner.getX(), inner.getY() + inner.getHeight() * (float) i / 5.0f,
                        inner.getRight(), inner.getY() + inner.getHeight() * (float) i / 5.0f, 1.0f);
        }

        const float thr = pv (proc, Params::dynThr);
        const float ratio = juce::jmax (1.0f, pv (proc, Params::dynRatio));

        juce::Path p;
        for (int i = 0; i <= 60; ++i)
        {
            const float t = (float) i / 60.0f;
            const float in = -60.0f + 60.0f * t;
            const float out = in <= thr ? in : thr + (in - thr) / ratio;

            const float x = inner.getX() + t * inner.getWidth();
            const float y = inner.getBottom() - juce::jlimit (0.0f, 1.0f, (out + 60.0f) / 60.0f)
                                               * inner.getHeight();
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }

        g.setColour (accent);
        g.strokePath (p, juce::PathStrokeType (1.7f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

        // gain-reduction strip on the right
        const float gr = proc.getGainReductionDb();
        auto bar = a.withLeft (a.getRight() - 26.0f).reduced (7.0f, 7.0f);
        g.setColour (juce::Colour (0xff111317));
        g.fillRoundedRectangle (bar, 2.0f);

        const float amount = juce::jlimit (0.0f, 1.0f, -gr / 18.0f);
        if (amount > 0.01f)
        {
            g.setColour (accent);
            g.fillRoundedRectangle (bar.withTop (bar.getBottom() - bar.getHeight() * amount), 2.0f);
        }
    }

    void sat (juce::Graphics& g, juce::Rectangle<float>)
    {
        // filled by the caller: a transfer curve inside the given band
    }

    void saturationCurve (juce::Graphics& g, juce::Rectangle<float> a)
    {
        g.setColour (juce::Colour (0xff090a0c));
        g.fillRoundedRectangle (a, 3.0f);
        g.setColour (juce::Colour (0xff262a30));
        g.drawRoundedRectangle (a.reduced (0.5f), 3.0f, 1.0f);

        auto inner = a.reduced (6.0f, 6.0f);
        juce::Path p;
        for (int i = 0; i < 120; ++i)
        {
            const float t = (float) i / 119.0f;
            const float x = inner.getX() + t * inner.getWidth();
            const float y = inner.getCentreY() - inner.getHeight() * 0.40f
                                * std::tanh (std::sin (t * 6.0f * juce::MathConstants<float>::pi)
                                             * 3.0f);
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (softAccent);
        g.strokePath (p, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    void decay (juce::Graphics& g, juce::Rectangle<float> a, MHSignatureAudioProcessor& proc)
    {
        g.setColour (juce::Colour (0xff090a0c));
        g.fillRoundedRectangle (a, 3.0f);
        g.setColour (juce::Colour (0xff262a30));
        g.drawRoundedRectangle (a.reduced (0.5f), 3.0f, 1.0f);

        auto inner = a.reduced (6.0f, 5.0f);
        const float rate = 6.0f / juce::jlimit (0.3f, 12.0f, pv (proc, Params::revDecay));

        juce::Path p;
        for (int i = 0; i < 90; ++i)
        {
            const float t = (float) i / 89.0f;
            const float x = inner.getX() + t * inner.getWidth();
            const float y = inner.getBottom() - std::exp (-t * rate) * inner.getHeight();
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (softAccent);
        g.strokePath (p, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    void delay (juce::Graphics& g, juce::Rectangle<float> a, MHSignatureAudioProcessor& proc)
    {
        g.setColour (juce::Colour (0xff090a0c));
        g.fillRoundedRectangle (a, 3.0f);
        g.setColour (juce::Colour (0xff262a30));
        g.drawRoundedRectangle (a.reduced (0.5f), 3.0f, 1.0f);

        auto inner = a.reduced (10.0f, 8.0f);
        const float fb = juce::jlimit (0.0f, 1.0f, pv (proc, Params::dlyFb) * 0.01f);

        for (int c = 0; c < 11; ++c)
        {
            const float amp = juce::jmax (0.07f, std::pow (0.72f, (float) c));
            const float x = inner.getX() + inner.getWidth() * (float) c / 10.0f;

            for (int r = 0; r < 2; ++r)
            {
                const float y = inner.getY() + inner.getHeight() * (0.32f + 0.36f * (float) r);
                const float rad = 1.4f + 1.8f * amp * (0.45f + 0.55f * fb);
                g.setColour (softAccent.withAlpha (juce::jlimit (0.06f, 0.95f, amp * 1.5f)));
                g.fillEllipse (x - rad, y - rad, rad * 2.0f, rad * 2.0f);
            }
        }
    }

    void lfo (juce::Graphics& g, juce::Rectangle<float> a, MHSignatureAudioProcessor& proc)
    {
        g.setColour (juce::Colour (0xff090a0c));
        g.fillRoundedRectangle (a, 3.0f);
        g.setColour (juce::Colour (0xff262a30));
        g.drawRoundedRectangle (a.reduced (0.5f), 3.0f, 1.0f);

        auto inner = a.reduced (6.0f, 6.0f);
        const float depth = juce::jlimit (0.0f, 1.0f, pv (proc, Params::modDepth) * 0.01f);
        const float mix = juce::jlimit (0.0f, 1.0f, pv (proc, Params::modMix) * 0.01f);

        juce::Path p;
        for (int i = 0; i < 100; ++i)
        {
            const float t = (float) i / 99.0f;
            const float x = inner.getX() + t * inner.getWidth();
            const float y = inner.getCentreY()
                          - inner.getHeight() * 0.34f
                            * std::sin (t * 4.0f * juce::MathConstants<float>::twoPi)
                            * (0.15f + 0.85f * depth) * (0.4f + 0.6f * mix);
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (softAccent);
        g.strokePath (p, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    void fan (juce::Graphics& g, juce::Rectangle<float> a, MHSignatureAudioProcessor& proc)
    {
        g.setColour (juce::Colour (0xff090a0c));
        g.fillRoundedRectangle (a, 3.0f);
        g.setColour (juce::Colour (0xff262a30));
        g.drawRoundedRectangle (a.reduced (0.5f), 3.0f, 1.0f);

        auto inner = a.reduced (8.0f, 4.0f);
        const float width = juce::jlimit (0.0f, 2.0f, pv (proc, Params::imgWidth) * 0.01f);
        const float bal = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::imgBalance) * 0.01f);

        const float cx = inner.getCentreX();
        const float cy = inner.getBottom();
        const float r = juce::jmin (inner.getWidth() * 0.44f, inner.getHeight() * 1.05f);

        juce::Path base;
        base.addCentredArc (cx, cy, r, r, 0.0f, -juce::MathConstants<float>::halfPi,
                            juce::MathConstants<float>::halfPi, true);
        g.setColour (juce::Colour (0xff3c414a));
        g.strokePath (base, juce::PathStrokeType (1.2f));

        const float spread = juce::jlimit (0.10f, 1.0f, width * 0.5f) * 1.1f;
        const float centre = -juce::MathConstants<float>::halfPi + bal * 0.5f;

        for (int i = -6; i <= 6; ++i)
        {
            const float ang = centre + spread * (float) i / 6.0f;
            const bool lit = std::abs (i) <= 2;

            g.setColour (lit ? accent : juce::Colour (0xff56565a));
            g.drawLine (cx + std::cos (ang) * r * 0.16f, cy + std::sin (ang) * r * 0.16f,
                        cx + std::cos (ang) * r * 0.95f, cy + std::sin (ang) * r * 0.95f,
                        lit ? 1.3f : 1.0f);
        }
    }

    void transient (juce::Graphics& g, juce::Rectangle<float> a, MHSignatureAudioProcessor& proc)
    {
        g.setColour (juce::Colour (0xff090a0c));
        g.fillRoundedRectangle (a, 3.0f);
        g.setColour (juce::Colour (0xff262a30));
        g.drawRoundedRectangle (a.reduced (0.5f), 3.0f, 1.0f);

        auto inner = a.reduced (6.0f, 5.0f);
        const float att = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::trnAtt) * 0.01f);
        const float sus = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::trnSus) * 0.01f);

        juce::Path p;
        for (int i = 0; i < 90; ++i)
        {
            const float t = (float) i / 89.0f;
            const float env = std::exp (-t * 5.0f) * (0.62f + 0.22f * att)
                            + (1.0f - std::exp (-t * 12.0f)) * 0.28f * (1.0f + sus);
            const float x = inner.getX() + t * inner.getWidth();
            const float y = inner.getBottom() - juce::jlimit (0.0f, 1.0f, env) * inner.getHeight();
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (softAccent);
        g.strokePath (p, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    void vo (juce::Graphics& g, juce::Rectangle<float> a, MHSignatureAudioProcessor& proc)
    {
        g.setColour (juce::Colour (0xff090a0c));
        g.fillRoundedRectangle (a, 3.0f);
        g.setColour (juce::Colour (0xff262a30));
        g.drawRoundedRectangle (a.reduced (0.5f), 3.0f, 1.0f);

        auto inner = a.reduced (6.0f, 5.0f);
        const float amt = juce::jlimit (0.0f, 1.0f, -pv (proc, Params::vocDeessR) / 18.0f);
        const float mid = inner.getCentreY();

        juce::Path p;
        for (int i = 0; i < 120; ++i)
        {
            const float t = (float) i / 119.0f;
            const float notch = std::exp (-0.5f * std::pow ((t - 0.72f) / 0.055f, 2.0f))
                                * (0.30f + 0.55f * amt);
            const float x = inner.getX() + t * inner.getWidth();
            const float y = mid - inner.getHeight() * 0.30f * (1.0f - notch)
                                - inner.getHeight() * 0.08f * std::sin (t * 9.0f);
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (softAccent);
        g.strokePath (p, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }
}

//==============================================================================
Canvas::Canvas (MHSignatureAudioProcessor& p) : proc (p)
{
    setOpaque (true);

    portrait = juce::ImageFileFormat::loadFrom (BinaryData::portrait_jpg,
                                                (size_t) BinaryData::portrait_jpgSize);

    tabs = std::make_unique<TabStrip>();
    tabs->onTabChanged = [this] (int i) { showPage ((Page) i); };
    addAndMakeVisible (*tabs);

    presetBar = std::make_unique<PresetBar> (proc);
    addAndMakeVisible (*presetBar);

    analyser = std::make_unique<AnalyserPanel> (proc);
    addAndMakeVisible (*analyser);

    rail = std::make_unique<ChainRail> (proc);
    addAndMakeVisible (*rail);

    // ------------------------------------------------------------- SIGNATURE ENGINE
    enginePanel = std::make_unique<ModulePanel> (proc, "SIGNATURE ENGINE");
    enginePanel->setReadout ("TURN IDEAS INTO SOUND");
    enginePanel->customLayout = [this]
    {
        if (enginePanel == nullptr) return;

        const int cx = enginePanel->getWidth() / 2;

        // the big dial: r = 48, so its block is 2r + 44 high and 2r + 26 wide
        if (engineKnob != nullptr)
            engineKnob->setBounds (cx - 61, 80, 122, 140);

        const int mx[4] { 45, 433, 45, 433 };
        const int my[4] { 46, 46, 142, 142 };

        for (int i = 0; i < (int) engineMacros.size() && i < 4; ++i)
            engineMacros[(size_t) i]->setBounds (mx[i], my[i], 62, 80);
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

        engineKnob = std::make_unique<MhKnob> (proc, Params::intensity, "INTENSITY", Fmt::pct0);
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
    eqPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        modart::eq (g, a, proc);
    });
    addAndMakeVisible (*eqPanel);

    // --------------------------------------------------------------- DYNAMICS
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
        modart::comp (g, a, proc);
    });
    addAndMakeVisible (*dynPanel);

    // --------------------------------------------------------------------- COLOUR
    satPanel = std::make_unique<ModulePanel> (proc, "COLOUR");
    satPanel->addPower (Params::satOn, "SAT");
    satPanel->addKnob (Params::satDrive, "DRIVE", Fmt::raw1);
    satPanel->addKnob (Params::satTone,  "TONE",  Fmt::pct0);
    satPanel->addKnob (Params::satMix,   "MIX",   Fmt::pct0);
    satPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        modart::saturationCurve (g, a);
    });
    addAndMakeVisible (*satPanel);

    satType = std::make_unique<ChoiceChip> (proc, Params::satType, "CHARACTER");
    satPanel->addAndMakeVisible (*satType);
    satPanel->setChipRow (true);              // the ChoiceChip below draws this row
    satPanel->customLayout = [this]
    {
        satType->setBounds (26, 46, juce::jmin (satPanel->getWidth() - 52, 132), 18);
    };
    satPanel->resized();

    // ---------------------------------------------------------------------- VOCAL
    vocPanel = std::make_unique<ModulePanel> (proc, "VOCAL CONTROL");
    vocPanel->addPower (Params::vocOn, "VOX");
    vocPanel->addKnob (Params::vocHpf,    "HPF",    Fmt::hz);
    vocPanel->addKnob (Params::vocBodyG,  "BODY",   Fmt::db1);
    vocPanel->addKnob (Params::vocDeessF, "DE-ESS", Fmt::hz);
    vocPanel->addKnob (Params::vocPresG,  "PRES",   Fmt::db1);
    vocPanel->addKnob (Params::vocAirG,   "AIR",    Fmt::db1);
    vocPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        modart::vo (g, a, proc);
    });
    addAndMakeVisible (*vocPanel);

    // --------------------------------------------------------------------- SPACE
    revPanel = std::make_unique<ModulePanel> (proc, "SPACE");
    revPanel->addPower (Params::revOn, "REVERB");
    revPanel->addPower (Params::dlyOn, "DELAY");
    revPanel->addKnob (Params::revDecay, "DECAY",  Fmt::sec1);
    revPanel->addKnob (Params::revPre,   "PREDLY", Fmt::ms);
    revPanel->addKnob (Params::revDamp,  "DAMP",   Fmt::pct0);
    revPanel->addKnob (Params::revMix,   "MIX",    Fmt::pct0);
    revPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        modart::decay (g, a.withWidth (a.getWidth() * 0.48f), proc);
        modart::delay (g, a.withTrimmedLeft (a.getWidth() * 0.52f), proc);
    });
    addAndMakeVisible (*revPanel);

    dlyPanel = std::make_unique<ModulePanel> (proc, "DELAY");
    dlyPanel->addPower (Params::dlyOn, "DLY");
    dlyPanel->addKnob (Params::dlyTime,   "TIME",   Fmt::ms);
    dlyPanel->addKnob (Params::dlyFb,     "FDBK",   Fmt::pct0);
    dlyPanel->addKnob (Params::dlyFilter, "FILTER", Fmt::hz);
    dlyPanel->addKnob (Params::dlyMix,    "MIX",    Fmt::pct0);
    dlyPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        modart::delay (g, a, proc);
    });
    addAndMakeVisible (*dlyPanel);

    // ------------------------------------------------------------------- MOTION
    modPanel = std::make_unique<ModulePanel> (proc, "MOTION");
    modPanel->addPower (Params::modOn, "MOD");
    modPanel->addPower (Params::trnOn, "TRNS");
    modPanel->addKnob (Params::modRate,  "RATE",    Fmt::raw1);
    modPanel->addKnob (Params::modDepth, "DEPTH",   Fmt::pct0);
    modPanel->addKnob (Params::modMix,   "MIX",     Fmt::pct0);
    modPanel->addKnob (Params::trnAtt,   "ATTACK",  Fmt::int0);
    modPanel->addKnob (Params::trnSus,   "SUSTAIN", Fmt::int0);
    modPanel->addKnob (Params::trnFreq,  "SPLIT",   Fmt::hz);
    modPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        modart::lfo (g, a.withWidth (a.getWidth() * 0.48f), proc);
        modart::transient (g, a.withTrimmedLeft (a.getWidth() * 0.52f), proc);
    });
    addAndMakeVisible (*modPanel);

    trnPanel = std::make_unique<ModulePanel> (proc, "TRANSIENT");
    trnPanel->addPower (Params::trnOn, "TRNS");
    trnPanel->addKnob (Params::trnAtt,  "ATTACK",  Fmt::int0);
    trnPanel->addKnob (Params::trnSus,  "SUSTAIN", Fmt::int0);
    trnPanel->addKnob (Params::trnFreq, "SPLIT",   Fmt::hz);
    trnPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        modart::transient (g, a, proc);
    });
    addAndMakeVisible (*trnPanel);

    // -------------------------------------------------------------------- IMAGER
    imgPanel = std::make_unique<ModulePanel> (proc, "STEREO IMAGER");
    imgPanel->addPower (Params::imgOn, "IMG");
    imgPanel->addKnob (Params::imgWidth,   "WIDTH", Fmt::pct0);
    imgPanel->addKnob (Params::imgBalance, "BAL",   Fmt::int0);
    imgPanel->addKnob (Params::imgMonoF,   "MONO",  Fmt::hz);
    imgPanel->setArt ([this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        modart::fan (g, a, proc);
    });
    addAndMakeVisible (*imgPanel);

    bandPanel = std::make_unique<ModulePanel> (proc, "WIDTH PER BAND");
    bandPanel->addPower (Params::imgOn, "IMG");
    bandPanel->addKnob (Params::imgWLow,  "LOW",  Fmt::pct0);
    bandPanel->addKnob (Params::imgWMid,  "MID",  Fmt::pct0);
    bandPanel->addKnob (Params::imgWHigh, "HIGH", Fmt::pct0);
    bandPanel->addKnob (Params::imgWAir,  "AIR",  Fmt::pct0);
    addAndMakeVisible (*bandPanel);

    // ------------------------------------------------------------------- UTILITY
    buildLayout();
    setSize (designW, designH);
    startTimerHz (12);
}

Canvas::~Canvas() = default;

//==============================================================================
void Canvas::showPage (Page p)
{
    page = p;
    tabs->setIndex ((int) p);
    buildLayout();
    repaint();
}

void Canvas::buildLayout()
{
    tabs->setBounds (496, 11, 560, 42);
    presetBar->setBounds (1120, 10, 232, 42);

    ModulePanel* const panels[] = { enginePanel.get(), eqPanel.get(), dynPanel.get(),
                                    satPanel.get(), vocPanel.get(), revPanel.get(),
                                    dlyPanel.get(), modPanel.get(), trnPanel.get(),
                                    imgPanel.get(), bandPanel.get() };

    for (auto* panel : panels)
    {
        if (panel == nullptr) continue;

        panel->setVisible (false);
        panel->setSections ({});          // the MOTION card re-adds its two captions
    }

    analyser->setVisible (true);
    rail->setVisible (true);
    rail->setBounds (1144, 70, 288, 778);

    const juce::Rectangle<int> hero  (212, 70, 608, 490);
    const juce::Rectangle<int> anal  (832, 70, 300, 490);
    const juce::Rectangle<int> compL (212, 572, 450, 276);
    const juce::Rectangle<int> compR (674, 572, 458, 276);

    // box, rows, max radius, artwork height — the numbers the render used
    auto place = [] (ModulePanel* p, juce::Rectangle<int> box, int rows, float maxR, float art)
    {
        if (p == nullptr) return;

        p->setArtHeight (art);
        p->setMaxRadius (maxR);
        p->setRows (rows);
        p->setBounds (box);
        p->setVisible (true);
    };

    switch (page)
    {
        case Page::all:
            enginePanel->setBounds (212, 70, 540, 230);
            enginePanel->setVisible (true);

            analyser->setBounds (764, 70, 368, 230);
            analyser->setStatsVisible (false);

            place (eqPanel.get(),  { 212, 312, 298, 270 }, 1, 22.0f,  54.0f);
            place (dynPanel.get(),  { 522, 312, 300, 270 }, 2, 22.0f,   0.0f);
            place (satPanel.get(),  { 834, 312, 298, 270 }, 1, 22.0f,  74.0f);
            place (vocPanel.get(),  { 212, 594, 298, 254 }, 1, 22.0f,  44.0f);
            place (revPanel.get(),  { 522, 594, 300, 254 }, 1, 22.0f,  40.0f);

            modPanel->setArtHeight (0.0f);
            modPanel->setMaxRadius (17.0f);
            modPanel->setSections ({ "MODULATION", "TRANSIENT" });
            modPanel->setBounds (834, 594, 298, 254);
            modPanel->setVisible (true);

            rail->setActive ("ENGINE");
            break;

        case Page::eq:
            place (eqPanel.get(),  hero, 1, 40.0f, 170.0f);
            place (vocPanel.get(), compL, 1, 19.0f, 60.0f);
            place (satPanel.get(), compR, 1, 19.0f, 60.0f);

            analyser->setBounds (anal);
            analyser->setStatsVisible (true);
            rail->setActive ("EQ");
            break;

        case Page::dyn:
            place (dynPanel.get(), hero,  1, 34.0f, 186.0f);
            place (trnPanel.get(), compL, 1, 19.0f,  60.0f);
            place (eqPanel.get(),  compR, 1, 19.0f,  60.0f);

            analyser->setBounds (anal);
            analyser->setStatsVisible (true);
            rail->setActive ("DYNAMICS");
            break;

        case Page::colour:
            place (satPanel.get(), hero,  1, 34.0f, 150.0f);
            place (vocPanel.get(), compL, 1, 19.0f,  60.0f);
            place (eqPanel.get(),  compR, 1, 19.0f,  60.0f);

            analyser->setBounds (anal);
            analyser->setStatsVisible (true);
            rail->setActive ("COLOUR");
            break;

        case Page::space:
            place (revPanel.get(), { 212, 70, 454, 490 }, 1, 32.0f, 190.0f);
            place (dlyPanel.get(), { 678, 70, 454, 490 }, 1, 32.0f, 190.0f);

            analyser->setBounds (212, 572, 920, 276);
            analyser->setStatsVisible (false);
            rail->setActive ("REVERB");
            break;

        case Page::mod:
            place (modPanel.get(), hero,  1, 34.0f, 150.0f);
            place (trnPanel.get(), compL, 1, 19.0f,  60.0f);
            place (revPanel.get(), compR, 1, 19.0f,  60.0f);

            analyser->setBounds (anal);
            analyser->setStatsVisible (true);
            rail->setActive ("MODULATION");
            break;

        case Page::image:
            place (imgPanel.get(),  hero,  1, 34.0f, 170.0f);
            place (bandPanel.get(), compL, 1, 19.0f,   0.0f);
            place (satPanel.get(),  compR, 1, 19.0f,  60.0f);

            analyser->setBounds (anal);
            analyser->setStatsVisible (true);
            rail->setActive ("IMAGER");
            break;
    }
}

//==============================================================================
void Canvas::timerCallback()
{
    rail->refresh();
    presetBar->refresh();

    ModulePanel* const all[] = { eqPanel.get(), dynPanel.get(), satPanel.get(),
                                 vocPanel.get(), revPanel.get(), dlyPanel.get(),
                                 modPanel.get(), trnPanel.get(), imgPanel.get(),
                                 bandPanel.get() };

    for (auto* p2 : all)
        if (p2 != nullptr && p2->isVisible())
            p2->refreshPowers();

    if (dynPanel != nullptr && dynPanel->isVisible())
        dynPanel->setReadout ("GR " + juce::String (proc.getGainReductionDb(), 1));

}

//==============================================================================
void Canvas::paint (juce::Graphics& g)
{
    g.fillAll (noir);

    // ------------------------------------------------------------- portrait rail
    const juce::Rectangle<float> railArea (8.0f, 66.0f, 192.0f, 792.0f);

    if (portrait.isValid())
    {
        g.drawImage (portrait, railArea, juce::RectanglePlacement::stretchToFit, false);

        g.setGradientFill (juce::ColourGradient (noir, railArea.getX(), railArea.getY(),
                                                 juce::Colours::transparentBlack,
                                                 railArea.getX(), railArea.getY() + 52.0f,
                                                 false));
        g.fillRect (railArea.withHeight (52.0f));

        const float fadeTop = railArea.getY() + railArea.getHeight() * 0.70f;
        g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack,
                                                 railArea.getX(), fadeTop, noir,
                                                 railArea.getX(), railArea.getBottom(), false));
        g.fillRect (railArea.withTop (fadeTop));

        g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack,
                                                 railArea.getRight() - 34.0f, 0.0f, noir,
                                                 railArea.getRight(), 0.0f, false));
        g.fillRect (railArea.withTrimmedLeft (railArea.getWidth() - 34.0f));
    }

    // signature
    {
        const float ccx = railArea.getCentreX();

        juce::Graphics::ScopedSaveState state (g);
        g.addTransform (juce::AffineTransform::rotation (-0.12f, ccx, 626.0f));
        g.setFont (juce::Font (juce::Font::getDefaultSerifFontName(), 20.0f, juce::Font::italic));
        g.setColour (juce::Colour (0xfff7f1eb));
        g.drawText ("Milad Hashemi", juce::Rectangle<float> (railArea.getX(), 612.0f,
                                                             railArea.getWidth(), 30.0f),
                    juce::Justification::centred, false);
        g.setOpacity (1.0f);
    }

    {
        juce::Path flourish;
        flourish.startNewSubPath (railArea.getX() + 44.0f, 656.0f);
        flourish.cubicTo (railArea.getX() + 72.0f, 650.0f, railArea.getX() + 120.0f, 654.0f,
                          railArea.getX() + 152.0f, 657.0f);
        g.setColour (accent.withAlpha (0.85f));
        g.strokePath (flourish, juce::PathStrokeType (1.2f, juce::PathStrokeType::curved));

        art::text (g, "MUSIC  \xc2\xb7  PEOPLE  \xc2\xb7  EMOTION",
                   { railArea.getX(), 669.0f, railArea.getWidth(), 14.0f },
                   6.2f, juce::Colour (0xffcbc3bd), juce::Justification::centred, false, 1.8f);
        art::text (g, "ALWAYS", { railArea.getX(), 685.0f, railArea.getWidth(), 14.0f },
                   6.2f, juce::Colour (0xffcbc3bd), juce::Justification::centred, false, 1.8f);

        art::text (g, "VERSION 1.0.0", { railArea.getX(), 789.0f, railArea.getWidth(), 14.0f },
                   6.2f, faint, juce::Justification::centred, false, 2.6f);
        art::text (g, "MH SIGNATURE", { railArea.getX(), 819.0f, railArea.getWidth(), 14.0f },
                   6.0f, juce::Colour (0xff80868f), juce::Justification::centred, false, 3.6f);

        g.setColour (juce::Colour (0xff22262c));
        g.drawLine (railArea.getRight() + 0.5f, railArea.getY(), railArea.getRight() + 0.5f,
                    railArea.getBottom(), 1.0f);
    }

    // -------------------------------------------------------------------- header
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff131518), 0.0f, 0.0f,
                                                 juce::Colour (0xff0c0d10), 0.0f, 61.0f, false));
        g.fillRect (juce::Rectangle<float> (0.0f, 0.0f, (float) designW, 61.0f));

        g.setColour (juce::Colour (0xff0a0b0c));
        g.drawLine (0.0f, 61.0f, (float) designW, 61.0f, 1.2f);
        g.setColour (juce::Colour (0xff34383f));
        g.drawLine (0.0f, 60.0f, (float) designW, 60.0f, 0.9f);

        // monogram
        {
            juce::Path mono;
            const float x = 22.0f, y = 17.0f, h = 27.0f, w = h * 0.40f;

            for (int k = 0; k < 2; ++k)
            {
                const float ox = x + (float) k * (w + h * 0.16f);
                mono.startNewSubPath (ox, y + h);
                mono.lineTo (ox, y);
                mono.lineTo (ox + w, y + h * 0.60f);
                mono.lineTo (ox + w, y + h);
            }

            g.setColour (ink);
            g.strokePath (mono, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved));

            g.setColour (accent);
            g.drawLine (x + w + h * 0.08f, y + h * 0.30f, x + w + h * 0.08f, y + h, 1.5f);
        }

        g.setColour (juce::Colour (0xff30343b));
        g.drawLine (82.0f, 12.0f, 82.0f, 50.0f, 1.0f);

        art::text (g, "MH SIGNATURE", { 94.0f, 14.0f, 320.0f, 22.0f }, 13.0f, ink,
                   juce::Justification::centredLeft, false, 4.0f);
        art::text (g, "BY MILAD HASHEMI", { 95.0f, 34.0f, 280.0f, 14.0f }, 6.8f, dimText,
                   juce::Justification::centredLeft, false, 3.2f);

        art::text (g, "SOUND WITH", { 1352.0f, 13.0f, 76.0f, 13.0f }, 6.4f, dimText,
                   juce::Justification::centredRight, false, 2.4f);
        art::text (g, "INTENTION", { 1352.0f, 27.0f, 76.0f, 13.0f }, 6.4f, dimText,
                   juce::Justification::centredRight, false, 2.4f);
    }

    // -------------------------------------------------------------------- footer
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0e0f12), 0.0f, 862.0f,
                                                 juce::Colour (0xff090a0c), 0.0f,
                                                 (float) designH, false));
        g.fillRect (juce::Rectangle<float> (0.0f, 862.0f, (float) designW,
                                            (float) designH - 862.0f));

        g.setColour (juce::Colour (0xff2e3239));
        g.drawLine (0.0f, 862.0f, (float) designW, 862.0f, 1.0f);

        art::text (g, "VERSION 1.0.0", { 212.0f, 873.0f, 200.0f, 14.0f }, 6.8f, faint,
                   juce::Justification::centredLeft, false, 2.6f);

        art::text (g, "AN ALL-IN-ONE MIXING ENVIRONMENT", { 522.0f, 873.0f, 600.0f, 14.0f },
                   8.6f, juce::Colour (0xffb0b7c0), juce::Justification::centred, false, 3.2f);

        art::text (g, "DESIGNED BY MILAD HASHEMI", { 1180.0f, 873.0f, 248.0f, 14.0f },
                   6.8f, faint, juce::Justification::centredRight, false, 2.6f);
    }
}

void Canvas::paintOverChildren (juce::Graphics&)
{
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
