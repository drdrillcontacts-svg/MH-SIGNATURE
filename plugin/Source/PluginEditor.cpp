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
    void fitText (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area,
                  float size, juce::Colour colour, juce::Justification just,
                  bool bold = false, float kerning = 1.2f)
    {
        auto font = uiFont (size, bold, kerning);
        const float needed = juce::GlyphArrangement::getStringWidth (font, text);

        if (needed > area.getWidth() && needed > 0.0f)
            font.setHeight (juce::jmax (7.0f, size * area.getWidth() / needed));

        g.setFont (font);
        g.setColour (colour);
        g.drawText (text, area, just, false);
    }

    void fillPanelGlass (juce::Graphics& g, juce::Rectangle<float> r, float corner)
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff15181d), r.getX(), r.getY(),
                                                 juce::Colour (0xff0a0c0f), r.getX(), r.getBottom(),
                                                 false));
        g.fillRoundedRectangle (r, corner);
        g.setColour (juce::Colour (0x0bffffff));
        g.fillRoundedRectangle (r.withHeight (r.getHeight() * 0.5f), corner);
        g.setColour (edgeCol);
        g.drawRoundedRectangle (r, corner, 1.0f);
    }
}

//==============================================================================
MhKnob::MhKnob (MHSignatureAudioProcessor& proc,
                const juce::String& paramId,
                const juce::String& labelText,
                Fmt format,
                bool bigStyle)
    : knobLabel (labelText), fmt (format), big (bigStyle)
{
    setSliderStyle (juce::Slider::RotaryVerticalDrag);
    setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    setWantsKeyboardFocus (false);
    setOpaque (false);
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    setMouseDragSensitivity (big ? 340 : 250);

    param = proc.apvts.getParameter (paramId);

    if (param != nullptr)
    {
        const auto def = param->convertFrom0to1 (param->getDefaultValue());
        setDoubleClickReturnValue (true, def);
        setTooltip (param->getName (64) + "  ·  double-click to reset");
    }

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, paramId, *this);
}

void MhKnob::setAccentGlow (float g)
{
    const float ng = juce::jlimit (0.0f, 1.0f, g);
    if (std::abs (ng - glow) > 0.02f) { glow = ng; repaint(); }
}

void MhKnob::setDimmed (bool shouldDim)
{
    if (dimmed == shouldDim) return;
    dimmed = shouldDim;
    setAlpha (dimmed ? 0.42f : 1.0f);
    repaint();
}

void MhKnob::mouseEnter (const juce::MouseEvent&) { hovered = true;  repaint(); }
void MhKnob::mouseExit  (const juce::MouseEvent&) { hovered = false; repaint(); }

void MhKnob::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    if (b.getWidth() < 12.0f || b.getHeight() < 12.0f) return;

    const float textH = big ? 34.0f : 27.0f;
    const float d = juce::jmin (b.getWidth() - 2.0f, b.getHeight() - textH);
    if (d < 18.0f) return;

    const float cx = b.getCentreX();
    const float cy = b.getY() + (b.getHeight() - textH - d) * 0.5f + d * 0.5f;
    const float r  = d * 0.5f - 1.0f;

    const float startRad = juce::MathConstants<float>::pi * 1.25f;
    const float endRad   = juce::MathConstants<float>::pi * 2.75f;
    const float sweep    = endRad - startRad;

    const float norm = (float) valueToProportionOfLength (getValue());
    const float stroke = big ? 3.8f : 2.4f;

    // track
    juce::Path track;
    track.addCentredArc (cx, cy, r - 1.8f, r - 1.8f, 0.0f, startRad, endRad, true);
    g.setColour (juce::Colour (0xff2b2f36));
    g.strokePath (track, juce::PathStrokeType (stroke, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    // value arc
    if (norm > 0.002f)
    {
        juce::Path arc;
        arc.addCentredArc (cx, cy, r - 1.8f, r - 1.8f, 0.0f, startRad,
                           startRad + sweep * norm, true);

        if (glow > 0.01f)
        {
            g.setColour (accent.withAlpha (0.20f * glow));
            g.strokePath (arc, juce::PathStrokeType (stroke * 4.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        }

        g.setColour (hovered ? softAccent : accent);
        g.strokePath (arc, juce::PathStrokeType (stroke, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    // body
    const float bodyR = r - (big ? 10.0f : 6.5f);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2d3037), cx, cy - bodyR,
                                             juce::Colour (0xff08090b), cx, cy + bodyR, false));
    g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0x30ffffff), cx - bodyR * 0.4f,
                                             cy - bodyR * 0.6f, juce::Colours::transparentBlack,
                                             cx + bodyR * 0.2f, cy + bodyR * 0.5f, true));
    g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    g.setColour (juce::Colour (0x16ffffff));
    g.drawEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

    // pointer
    const float angle = startRad + sweep * norm;
    const float r0 = bodyR * 0.28f;
    const float r1 = bodyR * 0.88f;

    g.setColour (juce::Colour (0x70000000));
    g.drawLine (cx + std::sin (angle) * r0, cy - std::cos (angle) * r0,
                cx + std::sin (angle) * r1, cy - std::cos (angle) * r1, big ? 3.4f : 2.6f);

    g.setColour (hovered ? softAccent : juce::Colour (0xffeae4e0));
    g.drawLine (cx + std::sin (angle) * r0, cy - std::cos (angle) * r0,
                cx + std::sin (angle) * r1, cy - std::cos (angle) * r1, big ? 2.2f : 1.7f);

    // text
    const float ty = cy + r + 4.0f;

    fitText (g, knobLabel,
             { b.getX() - 8.0f, ty, b.getWidth() + 16.0f, big ? 14.0f : 13.0f },
             big ? 11.0f : typeLabel,
             juce::Colour (0xffd9dee4), juce::Justification::centred, true, 1.6f);

    fitText (g, formatValue ((float) getValue(), fmt),
             { b.getX() - 8.0f, ty + (big ? 14.0f : 13.0f), b.getWidth() + 16.0f, 12.0f },
             big ? 11.0f : typeValue,
             softAccent, juce::Justification::centred, false, 0.4f);
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

    g.setColour (on ? juce::Colour (0xff1c1512) : juce::Colour (0xff15181d));
    g.fillRoundedRectangle (r, corner);

    g.setColour (on ? accent.withAlpha (0.75f)
                    : (over ? juce::Colour (0xff5b626c) : juce::Colour (0xff33373e)));
    g.drawRoundedRectangle (r, corner, 1.0f);

    auto dot = r.withWidth (r.getHeight()).reduced (r.getHeight() * 0.33f);
    if (on)
    {
        g.setColour (accent.withAlpha (0.28f));
        g.fillEllipse (dot.expanded (2.0f));
        g.setColour (down ? softAccent : accent);
        g.fillEllipse (dot);
    }
    else
    {
        g.setColour (juce::Colour (0xff4a505a));
        g.drawEllipse (dot, 1.2f);
    }

    g.setFont (uiFont (r.getHeight() * 0.44f, true, 1.6f));
    g.setColour (on ? juce::Colour (0xfff0e6e2) : (over ? juce::Colour (0xffb9c0c9)
                                                        : juce::Colour (0xff767e88)));
    g.drawText (on ? "ON" : "OFF",
                r.withTrimmedLeft (r.getHeight() + 2.0f).withTrimmedRight (6.0f),
                juce::Justification::centredLeft, false);
}

//==============================================================================
ModulePanel::ModulePanel (MHSignatureAudioProcessor& p, const juce::String& t)
    : title (t), proc (p)
{
    setInterceptsMouseClicks (true, true);
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
    k->onValueChange = [this] { if (artH > 0) repaint (getArtArea()); };
    knobs.push_back (std::move (k));
    resized();
}

void ModulePanel::setArt (int, std::function<void (juce::Graphics&, juce::Rectangle<float>)> drawer)
{
    artH = drawer != nullptr ? 1 : 0;
    artDrawer = std::move (drawer);
    resized();
    repaint();
}

void ModulePanel::setCaption (const juce::String& text, bool onTop)
{
    caption = text;
    captionOnTop = onTop;
    repaint();
}

juce::Rectangle<int> ModulePanel::headerArea() const
{
    return getLocalBounds().reduced (13, 0).withHeight (34);
}

juce::Rectangle<int> ModulePanel::getBodyArea() const
{
    auto r = getLocalBounds().reduced (13, 9);
    r.removeFromTop (26);
    return r;
}

juce::Rectangle<int> ModulePanel::getKnobArea() const
{
    auto body = getBodyArea();
    if (knobs.empty() || body.getHeight() < 30) return {};

    const int wanted = juce::jmin (body.getHeight(), 96);
    return artH == 0 ? body : body.removeFromBottom (wanted);
}

juce::Rectangle<int> ModulePanel::getArtArea() const
{
    auto body = getBodyArea();
    if (artH == 0 || body.getHeight() < 40) return {};

    body.removeFromBottom (getKnobArea().getHeight());
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
    // power pills, right aligned in the header
    const float ph = 20.0f;
    float px = (float) getWidth() - 13.0f;

    for (auto it = powers.rbegin(); it != powers.rend(); ++it)
    {
        const float w = juce::jlimit (46.0f, 66.0f, 24.0f + (float) (*it)->moduleName.length() * 6.0f);
        px -= w;
        (*it)->setBounds ((int) px, 8, (int) w, (int) ph);
        px -= 6.0f;
    }

    if (customLayout) customLayout();

    const int n = (int) knobs.size();
    if (n == 0) return;

    auto area = getKnobArea();
    if (area.getHeight() < 20 || area.getWidth() < 20) return;

    const int rows = n <= 6 ? 1 : 2;
    const int perRow = (n + rows - 1) / rows;
    const float slotW = (float) area.getWidth() / (float) perRow;
    const float slotH = (float) area.getHeight() / (float) rows;

    for (int i = 0; i < n; ++i)
    {
        const int r = i / perRow;
        const int c = i % perRow;
        knobs[(size_t) i]->setBounds (juce::Rectangle<float> (
            (float) area.getX() + (float) c * slotW,
            (float) area.getY() + (float) r * slotH,
            slotW, slotH).toNearestInt());
    }
}

void ModulePanel::paint (juce::Graphics& g)
{
    const bool live = isAnyModuleOn();
    const float alpha = focused ? 1.0f : 0.45f;

    g.setOpacity (alpha);
    fillPanelGlass (g, getLocalBounds().toFloat().reduced (0.5f), 8.0f);

    // header
    fitText (g, title.toUpperCase(), headerArea().toFloat(),
             typeModule, live ? juce::Colour (0xffe8ecf1) : juce::Colour (0xff7b828c),
             juce::Justification::centredLeft, true, 2.2f);

    // a hairline under the header keeps the card calm
    g.setColour (juce::Colour (0x12ffffff));
    g.drawLine (13.0f, 33.0f, (float) getWidth() - 13.0f, 33.0f, 1.0f);

    if (caption.isNotEmpty())
    {
        auto area = captionOnTop ? headerArea().toFloat()
                                 : getLocalBounds().toFloat().removeFromBottom (18.0f);
        fitText (g, caption, area,
                 typeSmall, juce::Colour (0xff818a95),
                 captionOnTop ? juce::Justification::centredRight
                              : juce::Justification::centred,
                 false, 2.4f);
    }

    if (artDrawer != nullptr && getArtArea().getHeight() > 16)
        artDrawer (g, getArtArea().toFloat());

    g.setOpacity (1.0f);

    // an off module reads as switched out of the chain
    if (! live)
    {
        g.setColour (juce::Colour (0x66050607));
        g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 8.0f);
    }
}

//==============================================================================
ChoiceChip::ChoiceChip (MHSignatureAudioProcessor& p, const juce::String& paramId,
                        const juce::String& cap)
    : proc (p), caption (cap)
{
    param = proc.apvts.getParameter (paramId);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setTooltip ("Click to change the saturation character");
}

void ChoiceChip::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (juce::Colour (0xff1a1d22));
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (juce::Colour (0xff333840));
    g.drawRoundedRectangle (r, 5.0f, 1.0f);

    juce::String text = "--";
    if (auto* ch = dynamic_cast<juce::AudioParameterChoice*> (param))
        text = ch->getCurrentChoiceName().toUpperCase();

    fitText (g, text, r.reduced (9.0f, 0.0f).withTrimmedRight (30.0f),
             typeSmall + 0.5f, juce::Colour (0xffe6eaf0), juce::Justification::centredLeft,
             true, 1.4f);

    g.setFont (uiFont (typeSmall - 2.0f, true, 1.2f));
    g.setColour (muted);
    g.drawText (caption.toUpperCase(), r.reduced (9.0f, 0.0f), juce::Justification::centredRight, false);
}

void ChoiceChip::mouseUp (const juce::MouseEvent&)
{
    if (auto* ch = dynamic_cast<juce::AudioParameterChoice*> (param))
    {
        const int next = (ch->getIndex() + 1) % juce::jmax (1, ch->choices.size());
        ch->beginChangeGesture();
        ch->setValueNotifyingHost (ch->convertTo0to1 ((float) next));
        ch->endChangeGesture();
        repaint();
    }
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

    auto readout = b.removeFromTop (22.0f);
    const float levelDb = isInput ? proc.getInputLevelDb() : proc.getOutputLevelDb();

    juce::String txt = levelDb <= -99.0f ? "-inf" : juce::String (levelDb, 1);
    fitText (g, txt + " dB", readout, 10.0f, juce::Colour (0xffe2e7ee),
             isInput ? juce::Justification::centredLeft : juce::Justification::centredRight,
             true, 0.8f);

    const float barW = 10.0f;
    juce::Rectangle<float> bar (isInput ? b.getX() + 10.0f : b.getRight() - 10.0f - barW,
                                b.getY() + 8.0f, barW, b.getHeight() - 30.0f);

    auto dbToY = [&] (float db)
    {
        const float n = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
        return bar.getBottom() - n * bar.getHeight();
    };

    g.setColour (juce::Colour (0xff0d0f13));
    g.fillRoundedRectangle (bar.expanded (2.0f, 1.0f), 2.5f);
    g.setColour (juce::Colour (0xff1f232a));
    g.drawRoundedRectangle (bar.expanded (2.0f, 1.0f), 2.5f, 1.0f);

    for (int db = 0; db >= -60; db -= 6)
    {
        const float y = dbToY ((float) db);
        g.setColour (juce::Colour (0x22ffffff));
        g.drawLine (isInput ? bar.getX() - 3.0f : bar.getRight(),
                    y, isInput ? bar.getX() - 1.0f : bar.getRight() + 2.0f, y, 1.0f);

        auto lr = juce::Rectangle<float> (isInput ? bar.getRight() + 6.0f : bar.getX() - 26.0f,
                                          y - 6.0f, 20.0f, 12.0f);
        fitText (g, juce::String (db), lr, 8.0f, juce::Colour (0xff5f6873),
                 isInput ? juce::Justification::centredLeft
                         : juce::Justification::centredRight, false, 0.2f);
    }

    const float n = juce::jlimit (0.0f, 1.0f, (levelDb + 60.0f) / 60.0f);
    if (n > 0.001f)
    {
        juce::Rectangle<float> fill (bar.getX(), dbToY (levelDb), bar.getWidth(),
                                     bar.getBottom() - dbToY (levelDb));

        g.setGradientFill (juce::ColourGradient (accent, bar.getX(), bar.getBottom(),
                                                 softAccent, bar.getX(), bar.getY(), false));
        g.fillRoundedRectangle (fill, 2.0f);

        g.setColour (softAccent);
        g.fillRect (bar.getX() - 3.0f, fill.getY() - 1.0f, bar.getWidth() + 6.0f, 2.0f);
    }
}

//==============================================================================
TabStrip::TabStrip()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

juce::Rectangle<float> TabStrip::tabBounds (int i) const
{
    const float w = (float) getWidth() / (float) items.size();
    return { (float) i * w + 3.0f, 3.0f, w - 6.0f, (float) getHeight() - 6.0f };
}

void TabStrip::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff0c0e12));
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    g.setColour (juce::Colour (0xff1d2026));
    g.drawRoundedRectangle (r.reduced (0.5f), r.getHeight() * 0.5f, 1.0f);

    for (int i = 0; i < items.size(); ++i)
    {
        auto tb = tabBounds (i);

        if (i == current)
        {
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe04a2a), tb.getX(), tb.getY(),
                                                     deepAccent, tb.getX(), tb.getBottom(), false));
            g.fillRoundedRectangle (tb, tb.getHeight() * 0.5f);
        }

        fitText (g, items[i], tb, typeSmall + 0.6f,
                 i == current ? juce::Colour (0xfff8e2db)
                              : (i == hovered ? juce::Colour (0xffe2e7ee) : muted),
                 juce::Justification::centred, true, 2.0f);
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
        if (tabBounds (i).contains (e.position))
        {
            current = i;
            repaint();
            if (onTabChanged) onTabChanged (current);
            return;
        }
}

//==============================================================================
PresetBar::PresetBar (MHSignatureAudioProcessor& p) : proc (p)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setTooltip ("Click to browse the 100 factory presets");
}

void PresetBar::mouseMove (const juce::MouseEvent& e)
{
    const int z = e.position.x < 28.0f ? -1
                                       : (e.position.x > (float) getWidth() - 28.0f ? 1 : 0);
    if (z != hoverZone) { hoverZone = z; repaint(); }
}

void PresetBar::mouseExit (const juce::MouseEvent&) { hoverZone = 0; repaint(); }

void PresetBar::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);

    fillPanelGlass (g, r, r.getHeight() * 0.5f);

    g.setFont (uiFont (13.0f, false, 0.0f));
    g.setColour (hoverZone == -1 ? softAccent : juce::Colour (0xff8b939e));
    g.drawText ("<", r.withWidth (28.0f), juce::Justification::centred, false);
    g.setColour (hoverZone == 1 ? softAccent : juce::Colour (0xff8b939e));
    g.drawText (">", r.withTrimmedLeft (r.getWidth() - 28.0f), juce::Justification::centred, false);

    auto txt = r.reduced (28.0f, 5.0f);
    auto top = txt.removeFromTop (txt.getHeight() * 0.42f);

    juce::String cat = MHSignatureAudioProcessor::getFactoryPresetCategory (proc.getCurrentPresetIndex());
    const int idx = proc.getCurrentPresetIndex() + 1;
    juce::String head = cat + "  ·  " + juce::String (idx).paddedLeft ('0', 3);
    if (proc.isPresetEdited()) head += "  *";

    fitText (g, head.toUpperCase(), top, typeSmall, juce::Colour (0xff8f97a2),
             juce::Justification::centred, false, 1.8f);

    fitText (g, proc.getCurrentPresetName().fromFirstOccurrenceOf (" ", false, false),
             txt, 13.5f, ink, juce::Justification::centred, false, 0.4f);
}

void PresetBar::mouseUp (const juce::MouseEvent& e)
{
    if (e.position.x < 28.0f)                        { proc.prevPreset(); repaint(); }
    else if (e.position.x > (float) getWidth() - 28.0f) { proc.nextPreset(); repaint(); }
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
//  Module artwork — drawn from the live parameter values, so every display is
//  a read-out and not decoration.
//==============================================================================
namespace art
{
    void grid (juce::Graphics& g, juce::Rectangle<float> a, int cols, int rows)
    {
        g.setColour (juce::Colour (0x14ffffff));
        for (int i = 1; i < cols; ++i)
            g.drawLine (a.getX() + a.getWidth() * (float) i / (float) cols, a.getY(),
                        a.getX() + a.getWidth() * (float) i / (float) cols, a.getBottom(), 1.0f);
        for (int i = 1; i < rows; ++i)
            g.drawLine (a.getX(), a.getY() + a.getHeight() * (float) i / (float) rows,
                        a.getRight(), a.getY() + a.getHeight() * (float) i / (float) rows, 1.0f);
    }

    void curve (juce::Graphics& g, juce::Rectangle<float> a,
                const std::function<float (float)>& fn, float alphaTop = 0.30f)
    {
        juce::Path p, fill;
        const int n = 130;

        for (int i = 0; i < n; ++i)
        {
            const float t = (float) i / (float) (n - 1);
            const float x = a.getX() + t * a.getWidth();
            const float y = a.getBottom() - juce::jlimit (0.0f, 1.0f, fn (t)) * a.getHeight();

            if (i == 0) { p.startNewSubPath (x, y); fill.startNewSubPath (x, a.getBottom()); }
            p.lineTo (x, y);
            fill.lineTo (x, y);
        }

        fill.lineTo (a.getRight(), a.getBottom());
        fill.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (alphaTop), 0.0f, a.getY(),
                                                 accent.withAlpha (0.015f), 0.0f, a.getBottom(), false));
        g.fillPath (fill);

        g.setColour (softAccent.withAlpha (0.92f));
        g.strokePath (p, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    void eq (MHSignatureAudioProcessor& proc, juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float lowG  = pv (proc, Params::eqLowG);
        const float lmG   = pv (proc, Params::eqLmG);
        const float hmG   = pv (proc, Params::eqHmG);
        const float highG = pv (proc, Params::eqHighG);
        const float tilt  = pv (proc, Params::eqTilt);
        const float hpf   = pv (proc, Params::eqHpf);

        grid (g, a, 4, 2);
        g.setColour (juce::Colour (0x20ffffff));
        g.drawLine (a.getX(), a.getCentreY(), a.getRight(), a.getCentreY(), 1.0f);

        auto bell = [] (float f, float fc, float oct)
        {
            const float d = std::log2 (juce::jmax (1.0f, f) / fc) / oct;
            return std::exp (-0.5f * d * d);
        };

        curve (g, a, [&] (float t)
        {
            const float f = 20.0f * std::pow (1000.0f, t);
            float db = lowG * bell (f, 65.0f, 1.15f)
                     + lmG  * bell (f, 152.0f, 0.85f)
                     + hmG  * bell (f, 3500.0f, 0.95f)
                     + highG * bell (f, 9000.0f, 1.5f)
                     + tilt * (t - 0.5f) * 6.0f;

            if (f < hpf)
                db -= 10.0f * std::log2 (hpf / juce::jmax (20.0f, f));

            return (db + 14.0f) / 28.0f;
        });
    }

    void compressor (MHSignatureAudioProcessor& proc, juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float gr = proc.getGainReductionDb();
        auto left  = a.withWidth (a.getWidth() * 0.34f).reduced (0.0f, 2.0f);
        auto right = a.withTrimmedLeft (a.getWidth() * 0.40f).reduced (0.0f, 2.0f);

        g.setColour (juce::Colour (0xff15181d));
        g.fillRoundedRectangle (left, 3.0f);
        g.setColour (juce::Colour (0xff1e2229));
        g.drawRoundedRectangle (left, 3.0f, 1.0f);

        const float thr = pv (proc, Params::dynThr);
        const float ratio = juce::jmax (1.0f, pv (proc, Params::dynRatio));

        juce::Path io;
        for (int i = 0; i <= 44; ++i)
        {
            const float t = (float) i / 44.0f;
            const float in = -60.0f + 60.0f * t;
            const float out = in <= thr ? in : thr + (in - thr) / ratio;
            const float x = left.getX() + 3.0f + t * (left.getWidth() - 6.0f);
            const float y = left.getBottom() - 3.0f
                          - juce::jlimit (0.0f, 1.0f, (out + 60.0f) / 60.0f) * (left.getHeight() - 6.0f);
            if (i == 0) io.startNewSubPath (x, y); else io.lineTo (x, y);
        }
        g.setColour (accent.withAlpha (0.9f));
        g.strokePath (io, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

        fitText (g, "GAIN REDUCTION", right.withHeight (12.0f), typeSmall - 1.5f,
                 juce::Colour (0xff78808b), juce::Justification::centredLeft, true, 1.6f);

        auto bar = right.withTrimmedTop (14.0f).withHeight (juce::jmin (16.0f, right.getHeight() - 14.0f));
        g.setColour (juce::Colour (0xff0c0e12));
        g.fillRoundedRectangle (bar, 3.0f);

        const float amount = juce::jlimit (0.0f, 1.0f, -gr / 12.0f);
        if (amount > 0.004f)
        {
            g.setGradientFill (juce::ColourGradient (accent, bar.getX(), bar.getCentreY(),
                                                     softAccent, bar.getRight(), bar.getCentreY(), false));
            g.fillRoundedRectangle (bar.withWidth (juce::jmax (4.0f, bar.getWidth() * amount)), 3.0f);
        }

        fitText (g, gr < -0.05f ? juce::String (gr, 1) + " dB" : "0.0 dB",
                 bar.reduced (6.0f, 0.0f), 9.5f, juce::Colour (0xfff2eee9),
                 juce::Justification::centredRight, true, 0.4f);
    }

    void saturation (MHSignatureAudioProcessor& proc, juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float drive = juce::jlimit (0.0f, 1.0f, pv (proc, Params::satDrive) / 18.0f);

        juce::Path p;
        const int n = 120;
        for (int i = 0; i < n; ++i)
        {
            const float t = (float) i / (float) (n - 1);
            const float x = a.getX() + t * a.getWidth();
            const float s = std::sin (t * juce::MathConstants<float>::twoPi * 3.0f);
            const float y = a.getCentreY()
                          - std::tanh (s * (1.0f + drive * 6.0f)) * a.getHeight() * 0.42f;
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }

        g.setColour (softAccent.withAlpha (0.92f));
        g.strokePath (p, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    }

    void reverb (MHSignatureAudioProcessor& proc, juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float decay = juce::jlimit (0.3f, 12.0f, pv (proc, Params::revDecay));
        const float damp = juce::jlimit (0.0f, 1.0f, pv (proc, Params::revDamp) * 0.01f);
        const float mix = juce::jlimit (0.0f, 1.0f, pv (proc, Params::revMix) * 0.01f);

        curve (g, a, [&] (float t)
        {
            const float e = std::exp (-t * (6.0f / decay) * (1.0f + damp * 1.4f));
            return e * (0.35f + 0.6f * mix);
        });
    }

    void delay (MHSignatureAudioProcessor& proc, juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float fb = juce::jlimit (0.0f, 1.0f, pv (proc, Params::dlyFb) * 0.01f);
        const int cols = 9, rows = 3;
        const float dx = a.getWidth() / (float) (cols - 1);
        const float dy = a.getHeight() / (float) (rows + 1);

        for (int c = 0; c < cols; ++c)
        {
            const float amp = std::pow (juce::jmax (0.02f, fb), (float) c * 0.6f);
            for (int r = 0; r < rows; ++r)
            {
                const bool alt = ((c + r) % 2) == 0;
                const float alpha = juce::jlimit (0.05f, 0.95f, amp * (alt ? 1.0f : 0.5f));
                const float d = 2.4f + 2.2f * amp;
                g.setColour ((alt ? accent : deepAccent).withAlpha (alpha));
                g.fillEllipse (a.getX() + (float) c * dx - d * 0.5f,
                               a.getY() + dy * (float) (r + 1) - d * 0.5f, d, d);
            }
        }
    }

    void modulation (MHSignatureAudioProcessor& proc, juce::Graphics& g, juce::Rectangle<float> a,
                     float phase)
    {
        const float depth = juce::jlimit (0.0f, 1.0f, pv (proc, Params::modDepth) * 0.01f);
        const float mix = juce::jlimit (0.0f, 1.0f, pv (proc, Params::modMix) * 0.01f);

        curve (g, a, [&] (float t)
        {
            return 0.5f - std::sin (t * juce::MathConstants<float>::twoPi
                                    + phase * juce::MathConstants<float>::twoPi)
                        * 0.42f * (0.15f + 0.85f * depth) * (0.4f + 0.6f * mix);
        }, 0.22f);
    }

    void imager (MHSignatureAudioProcessor& proc, juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float width = juce::jlimit (0.0f, 2.0f, pv (proc, Params::imgWidth) * 0.01f);
        const float bal = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::imgBalance) * 0.01f);

        const juce::Point<float> c (a.getCentreX(), a.getBottom() - 2.0f);
        const float r = juce::jmin (a.getWidth() * 0.46f, a.getHeight() - 4.0f);

        juce::Path arc;
        arc.addCentredArc (c.x, c.y, r, r, 0.0f, juce::MathConstants<float>::pi,
                           juce::MathConstants<float>::twoPi, true);
        g.setColour (juce::Colour (0xff282c33));
        g.strokePath (arc, juce::PathStrokeType (1.3f));

        const float spread = juce::jlimit (0.08f, 1.0f, width * 0.5f)
                           * juce::MathConstants<float>::halfPi;
        const float rot = bal * 0.5f;

        juce::Path petal;
        petal.startNewSubPath (c.x, c.y);
        petal.addCentredArc (c.x, c.y, r * 0.96f, r * 0.96f, 0.0f,
                             -juce::MathConstants<float>::halfPi - rot - spread * 0.5f,
                             -juce::MathConstants<float>::halfPi - rot + spread * 0.5f, true);
        petal.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.62f), c.x, c.y,
                                                 deepAccent.withAlpha (0.04f), c.x, c.y - r, true));
        g.fillPath (petal);

        g.setColour (accent.withAlpha (0.5f));
        g.drawLine (c.x, c.y, c.x + std::sin (rot) * r * 0.96f,
                    c.y - std::cos (rot) * r * 0.96f, 1.2f);

        g.setColour (juce::Colour (0xff3a3f46));
        g.drawLine (c.x, c.y, c.x - r, c.y, 1.0f);
        g.drawLine (c.x, c.y, c.x + r, c.y, 1.0f);
    }

    void transient (MHSignatureAudioProcessor& proc, juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float att = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::trnAtt) * 0.01f);
        const float sus = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::trnSus) * 0.01f);

        curve (g, a, [&] (float t)
        {
            const float env = std::exp (-t * 4.0f) * (0.55f + 0.45f * att)
                            + (1.0f - std::exp (-t * 10.0f)) * 0.25f * (1.0f + sus);
            return env;
        });
    }
}

//==============================================================================
class SpectrumPanel : public ModulePanel
{
public:
    explicit SpectrumPanel (MHSignatureAudioProcessor& p)
        : ModulePanel (p, juce::String()), proc (p)
    {
        setMouseCursor (juce::MouseCursor::NormalCursor);

        for (auto& v : hist) v = -90.0f;
    }

    void tick()
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

    void paint (juce::Graphics& g) override
    {
        ModulePanel::paint (g);

        // tabs
        for (int i = 0; i < tabNames.size(); ++i)
        {
            auto tb = tabRect (i);

            if (i == mode)
            {
                g.setColour (accent.withAlpha (0.20f));
                g.fillRoundedRectangle (tb.expanded (4.0f, 1.0f), 8.0f);
            }

            fitText (g, tabNames[i], tb, typeSmall,
                     i == mode ? juce::Colour (0xfff6ddd6) : juce::Colour (0xff7c848e),
                     juce::Justification::centred, true, 1.8f);
        }

        // pre/post toggle
        auto chip = chipRect();
        g.setColour (juce::Colour (0xff15181d));
        g.fillRoundedRectangle (chip, 8.0f);
        g.setColour (pre ? accent.withAlpha (0.7f) : juce::Colour (0xff2c3037));
        g.drawRoundedRectangle (chip, 8.0f, 1.0f);
        fitText (g, pre ? "PRE" : "POST", chip, typeSmall - 0.5f,
                 pre ? softAccent : juce::Colour (0xff9198a2),
                 juce::Justification::centred, true, 1.6f);

        // plot
        auto plot = plotArea();
        g.setColour (juce::Colour (0xff080a0d));
        g.fillRoundedRectangle (plot, 5.0f);

        auto inner = plot.reduced (2.0f).withTrimmedLeft (22.0f).withTrimmedBottom (16.0f);

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
            fitText (g, juce::String (db),
                     { plot.getX() + 2.0f, y - 6.0f, 19.0f, 12.0f }, 7.5f,
                     juce::Colour (0xff5d6570), juce::Justification::centredRight, false, 0.2f);
        }

        const float freqs[]  { 20.0f, 100.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f, 20000.0f };
        const char* labels[] { "20", "100", "500", "1k", "5k", "10k", "20k" };

        for (int i = 0; i < 7; ++i)
        {
            const float x = freqToX (freqs[i]);
            g.setColour (gridCol);
            g.drawLine (x, inner.getY(), x, inner.getBottom(), 1.0f);
            fitText (g, labels[i], { x - 14.0f, inner.getBottom() + 1.0f, 28.0f, 12.0f }, 7.5f,
                     juce::Colour (0xff5d6570), juce::Justification::centred, false, 0.2f);
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

            g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.50f), 0.0f, inner.getY(),
                                                     accent.withAlpha (0.02f), 0.0f, inner.getBottom(), false));
            g.fillPath (fill);

            g.setColour (softAccent.withAlpha (0.95f));
            g.strokePath (line, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
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

        g.setColour (juce::Colour (0xff1a1e24));
        g.drawRoundedRectangle (plot, 5.0f, 1.0f);
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        for (int i = 0; i < tabNames.size(); ++i)
            if (tabRect (i).expanded (4.0f, 1.0f).contains (e.position))
            {
                mode = i;
                repaint();
                return;
            }

        if (chipRect().contains (e.position))
        {
            pre = ! pre;
            repaint();
            return;
        }

    }

private:
    juce::Rectangle<float> tabRect (int i) const
    {
        const float widths[] { 74.0f, 78.0f, 74.0f };
        float x = (float) getBodyArea().getX() + 2.0f;
        for (int k = 0; k < i; ++k) x += widths[k];
        return { x, (float) getBodyArea().getY(), widths[i], 18.0f };
    }

    juce::Rectangle<float> chipRect() const
    {
        auto body = getBodyArea();
        return { (float) body.getRight() - 46.0f, (float) body.getY(), 46.0f, 18.0f };
    }

    juce::Rectangle<float> plotArea() const
    {
        auto body = getBodyArea();
        body.removeFromTop (22);
        return body.toFloat().translated (0.0f, 2.0f);
    }

    MHSignatureAudioProcessor& proc;
    int mode = 0;
    bool pre = false;

    juce::StringArray tabNames { "SPECTRUM", "WAVE", "LEVEL" };

    std::array<float, MHSignatureAudioProcessor::numSpectrumBands> spec {}, specPre {};
    std::array<float, MHSignatureAudioProcessor::numScopeSamples> wave {};
    std::array<float, 128> hist {};
    int histIndex = 0, loudCounter = 0;
    float loudAccum = -90.0f;
};

//==============================================================================
Canvas::Canvas (MHSignatureAudioProcessor& p) : proc (p)
{
    setOpaque (true);

    portrait = juce::ImageFileFormat::loadFrom (BinaryData::portrait_jpg,
                                                (size_t) BinaryData::portrait_jpgSize);

    // ---- meters ----------------------------------------------------------
    inMeter  = std::make_unique<LevelMeter> (proc, true);
    outMeter = std::make_unique<LevelMeter> (proc, false);
    addAndMakeVisible (*inMeter);
    addAndMakeVisible (*outMeter);

    // ---- header ----------------------------------------------------------
    tabs = std::make_unique<TabStrip>();
    tabs->onTabChanged = [this] (int i) { applyFocus (i); };
    addAndMakeVisible (*tabs);

    presetBar = std::make_unique<PresetBar> (proc);
    addAndMakeVisible (*presetBar);

    // ---- spectrum --------------------------------------------------------
    spectrumPanel = std::make_unique<SpectrumPanel> (proc);
    addAndMakeVisible (*spectrumPanel);

    // ---- SIGNATURE ENGINE ------------------------------------------------
    enginePanel = std::make_unique<ModulePanel> (proc, "SIGNATURE ENGINE");
    enginePanel->setCaption ("TURN IDEAS INTO SOUND", true);
    enginePanel->customLayout = [this]
    {
        auto b = enginePanel->getLocalBounds();
        auto left = b.reduced (14, 0).withTrimmedTop (40).withTrimmedBottom (12);
        left.setWidth (juce::jmin (left.getWidth() - 210, 290));

        const int colW = left.getWidth() / 2;
        const int rowH = left.getHeight() / 2;

        for (int i = 0; i < (int) engineMacros.size(); ++i)
        {
            const int c = i % 2, r = i / 2;
            engineMacros[(size_t) i]->setBounds (left.getX() + c * colW,
                                                 left.getY() + r * rowH, colW, rowH);
        }

        if (engineKnob != nullptr)
        {
            const int kw = juce::jmin (200, b.getWidth() - left.getWidth() - 62);
            const int kh = b.getHeight() - 52;
            engineKnob->setBounds (b.getWidth() - kw - 22, 36, kw, kh);
        }
    };
    addAndMakeVisible (*enginePanel);

    {
        auto addMacro = [this] (const char* id, const char* text)
        {
            auto k = std::make_unique<MhKnob> (proc, id, text, Fmt::int0);
            enginePanel->addAndMakeVisible (*k);
            engineMacros.push_back (std::move (k));
        };

        addMacro (Params::character, "CHARACTER");
        addMacro (Params::density,   "DENSITY");
        addMacro (Params::space,     "SPACE");
        addMacro (Params::width,     "WIDTH");

        engineKnob = std::make_unique<MhKnob> (proc, Params::intensity, "INTENSITY", Fmt::pct0, true);
        enginePanel->addAndMakeVisible (*engineKnob);
        enginePanel->resized();
    }

    // ---- EQ --------------------------------------------------------------
    eqPanel = std::make_unique<ModulePanel> (proc, "EQ");
    eqPanel->addPower (Params::eqOn, "EQ");
    eqPanel->addKnob (Params::eqHpf,   "HPF",   Fmt::hz);
    eqPanel->addKnob (Params::eqLowG,  "LOW",   Fmt::db1);
    eqPanel->addKnob (Params::eqLmG,   "LO MID",Fmt::db1);
    eqPanel->addKnob (Params::eqHmG,   "HI MID",Fmt::db1);
    eqPanel->addKnob (Params::eqHighG, "HIGH",  Fmt::db1);
    eqPanel->setArt (40, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::eq (proc, g, a);
    });
    addAndMakeVisible (*eqPanel);

    // ---- DYNAMICS + LIMITER ---------------------------------------------
    dynPanel = std::make_unique<ModulePanel> (proc, "DYNAMICS");
    dynPanel->addPower (Params::dynOn, "COMP");
    dynPanel->addPower (Params::limOn, "LIMIT");
    dynPanel->addKnob (Params::dynThr,    "THRESH", Fmt::db1);
    dynPanel->addKnob (Params::dynRatio,  "RATIO",  Fmt::ratio);
    dynPanel->addKnob (Params::dynAtt,    "ATTACK", Fmt::ms);
    dynPanel->addKnob (Params::dynRel,    "RELEASE",Fmt::ms);
    dynPanel->addKnob (Params::dynMakeup, "MAKEUP", Fmt::db1);
    dynPanel->addKnob (Params::limCeiling,"CEILING",Fmt::db1);
    dynPanel->setArt (40, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::compressor (proc, g, a);
    });
    addAndMakeVisible (*dynPanel);

    // ---- SATURATION · COLOR ---------------------------------------------
    satPanel = std::make_unique<ModulePanel> (proc, "COLOR");
    satPanel->addPower (Params::satOn, "SAT");
    satPanel->addKnob (Params::satDrive, "DRIVE", Fmt::db1);
    satPanel->addKnob (Params::satTone,  "TONE",  Fmt::pct0);
    satPanel->addKnob (Params::satMix,   "MIX",   Fmt::pct0);
    satPanel->customLayout = [this]
    {
        const auto body = satPanel->getBodyArea();
        if (satType != nullptr)
            satType->setBounds (body.getX(), body.getY() + 2,
                                juce::jmin (body.getWidth(), 118), 20);
    };
    addAndMakeVisible (*satPanel);

    satType = std::make_unique<ChoiceChip> (proc, Params::satType, "TYPE");
    satPanel->addAndMakeVisible (*satType);
    satPanel->resized();

    // ---- VOCAL CONTROL ---------------------------------------------------
    vocPanel = std::make_unique<ModulePanel> (proc, "VOCAL");
    vocPanel->addPower (Params::vocOn, "VOX");
    vocPanel->addKnob (Params::vocHpf,    "HPF",   Fmt::hz);
    vocPanel->addKnob (Params::vocBodyG,  "BODY",  Fmt::db1);
    vocPanel->addKnob (Params::vocDeessF, "DE-ESS",Fmt::hz);
    vocPanel->addKnob (Params::vocPresG,  "PRES",  Fmt::db1);
    vocPanel->addKnob (Params::vocAirG,   "AIR",   Fmt::db1);
    addAndMakeVisible (*vocPanel);

    // ---- DELAY + REVERB --------------------------------------------------
    spacePanel = std::make_unique<ModulePanel> (proc, "DELAY · REVERB");
    spacePanel->addPower (Params::dlyOn, "DLY");
    spacePanel->addPower (Params::revOn, "REV");
    spacePanel->addKnob (Params::dlyTime,  "TIME",  Fmt::ms);
    spacePanel->addKnob (Params::dlyFb,    "FDBK",  Fmt::pct0);
    spacePanel->addKnob (Params::dlyMix,   "DLY MIX", Fmt::pct0);
    spacePanel->addKnob (Params::revDecay, "DECAY", Fmt::sec1);
    spacePanel->addKnob (Params::revDamp,  "DAMP",  Fmt::pct0);
    spacePanel->addKnob (Params::revMix,   "REV MIX", Fmt::pct0);
    spacePanel->setArt (40, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::delay (proc, g, a.withWidth (a.getWidth() * 0.46f));
        art::reverb (proc, g, a.withTrimmedLeft (a.getWidth() * 0.52f));
    });
    addAndMakeVisible (*spacePanel);

    // ---- STEREO IMAGER ---------------------------------------------------
    imgPanel = std::make_unique<ModulePanel> (proc, "IMAGE");
    imgPanel->addPower (Params::imgOn, "IMG");
    imgPanel->addKnob (Params::imgWidth,   "WIDTH",   Fmt::pct0);
    imgPanel->addKnob (Params::imgBalance, "BALANCE", Fmt::int0);
    imgPanel->setArt (40, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::imager (proc, g, a);
    });
    addAndMakeVisible (*imgPanel);

    // ---- MODULATION ------------------------------------------------------
    modPanel = std::make_unique<ModulePanel> (proc, "MODULATION");
    modPanel->addPower (Params::modOn, "MOD");
    modPanel->addKnob (Params::modRate,  "RATE",  Fmt::raw1);
    modPanel->addKnob (Params::modDepth, "DEPTH", Fmt::pct0);
    modPanel->addKnob (Params::modMix,   "MIX",   Fmt::pct0);
    modPanel->setArt (26, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::modulation (proc, g, a, lfoPhase);
    });
    addAndMakeVisible (*modPanel);

    // ---- TRANSIENT -------------------------------------------------------
    trnPanel = std::make_unique<ModulePanel> (proc, "TRANSIENT");
    trnPanel->addPower (Params::trnOn, "TRNS");
    trnPanel->addKnob (Params::trnAtt,  "ATTACK",  Fmt::int0);
    trnPanel->addKnob (Params::trnSus,  "SUSTAIN", Fmt::int0);
    trnPanel->addKnob (Params::trnFreq, "SPLIT",   Fmt::hz);
    trnPanel->setArt (26, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        art::transient (proc, g, a);
    });
    addAndMakeVisible (*trnPanel);

    // ---- I/O -------------------------------------------------------------
    ioPanel = std::make_unique<ModulePanel> (proc, "I/O");
    ioPanel->addKnob (Params::ioInput,  "INPUT",  Fmt::db1);
    ioPanel->addKnob (Params::ioOutput, "OUTPUT", Fmt::db1);
    ioPanel->addKnob (Params::ioMix,    "MIX",    Fmt::pct0);
    addAndMakeVisible (*ioPanel);

    monoSwitch = std::make_unique<PowerSwitch> (proc, Params::ioMono, "MONO");
    bypassSwitch = std::make_unique<PowerSwitch> (proc, Params::ioBypass, "BYPASS");
    ioPanel->addAndMakeVisible (*monoSwitch);
    ioPanel->addAndMakeVisible (*bypassSwitch);
    ioPanel->customLayout = [this]
    {
        auto b = ioPanel->getBodyArea();
        const int w = (b.getWidth() - 8) / 2;
        monoSwitch->setBounds (b.getX(), b.getY() + 4, w, 22);
        bypassSwitch->setBounds (b.getX() + w + 8, b.getY() + 4, w, 22);
    };
    ioPanel->resized();

    buildLayout();
    setSize (designW, designH);
    startTimerHz (24);
}

Canvas::~Canvas() = default;

//==============================================================================
void Canvas::buildLayout()
{
    // far-left / far-right meters, portrait rail, then the module grid
    inMeter->setBounds  (16, 78, 56, 780);
    outMeter->setBounds (1368, 78, 56, 780);

    tabs->setBounds (532, 18, 400, 34);
    presetBar->setBounds (948, 14, 288, 42);

    enginePanel->setBounds   (448, 78, 560, 226);
    spectrumPanel->setBounds (1016, 78, 344, 226);

    eqPanel->setBounds  (448, 314, 300, 186);
    dynPanel->setBounds (760, 314, 372, 186);
    satPanel->setBounds (1144, 314, 216, 186);

    vocPanel->setBounds   (448, 510, 300, 186);
    spacePanel->setBounds (760, 510, 372, 186);
    imgPanel->setBounds   (1144, 510, 216, 186);

    modPanel->setBounds (448, 706, 260, 148);
    trnPanel->setBounds (720, 706, 288, 148);
    ioPanel->setBounds  (1020, 706, 340, 148);

    applyFocus (tabs->getIndex());
}

void Canvas::applyFocus (int tab)
{
    auto focus = [] (ModulePanel* p, bool on) { p->setFocused (on); };

    focus (eqPanel,  tab == 0 || tab == 1);
    focus (dynPanel, tab == 0 || tab == 2);
    focus (vocPanel, tab == 0 || tab == 2);
    focus (trnPanel, tab == 0 || tab == 2);
    focus (satPanel, tab == 0 || tab == 3);
    focus (spacePanel, tab == 0 || tab == 4);
    focus (modPanel, tab == 0 || tab == 5);
    focus (imgPanel, tab == 0 || tab == 6);
}

void Canvas::timerCallback()
{
    lfoPhase += 0.0065f;
    if (lfoPhase > 1.0f) lfoPhase -= 1.0f;

    engineGlow = juce::jmin (1.0f, 0.12f + proc.getEngineActivity() * 1.4f);
    if (engineKnob != nullptr) engineKnob->setAccentGlow (engineGlow);

    inMeter->repaint();
    outMeter->repaint();
    spectrumPanel->tick();

    dynPanel->repaint (dynPanel->getArtArea());
    modPanel->repaint (modPanel->getArtArea());
    imgPanel->repaint (imgPanel->getArtArea());
    spacePanel->repaint (spacePanel->getArtArea());
    presetBar->repaint();
}

//==============================================================================
void Canvas::paint (juce::Graphics& g)
{
    g.fillAll (noir);

    // ------------------------------------------------------------- portrait
    const juce::Rectangle<float> rail (76.0f, 74.0f, 364.0f, 784.0f);
    const juce::Rectangle<float> photo (rail.getX(), rail.getY(), rail.getWidth(), 566.0f);

    if (portrait.isValid())
    {
        const float iw = (float) portrait.getWidth();
        const float ih = (float) portrait.getHeight();
        const float s = juce::jmax (photo.getWidth() / iw, photo.getHeight() / ih);
        const float w = iw * s, h = ih * s;

        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (photo.toNearestInt());
        g.drawImage (portrait, juce::Rectangle<float> (photo.getCentreX() - w * 0.5f,
                                                       photo.getY(), w, h),
                     juce::RectanglePlacement::stretchToFit, false);
    }
    else
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a1108), 0.0f, 0.0f,
                                                 noir, photo.getX(), photo.getBottom(), false));
        g.fillRect (photo);
    }

    // fades so the portrait melts into the interface
    {
        g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, 0.0f, 430.0f,
                                                 noir, 0.0f, 640.0f, false));
        g.fillRect (photo.withTop (430.0f).withBottom (640.0f));

        g.setGradientFill (juce::ColourGradient (noir.withAlpha (0.92f), photo.getX(), 0.0f,
                                                 juce::Colours::transparentBlack, photo.getX() + 54.0f,
                                                 0.0f, false));
        g.fillRect (photo.withWidth (54.0f));

        g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack,
                                                 photo.getRight() - 90.0f, 0.0f, noir,
                                                 photo.getRight(), 0.0f, false));
        g.fillRect (photo.withTrimmedLeft (photo.getWidth() - 90.0f));
    }

    // vermilion bloom behind the engine
    g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.06f), 448.0f, 90.0f,
                                             juce::Colours::transparentBlack, 820.0f, 320.0f, true));
    g.fillRect (juce::Rectangle<float> (440.0f, 70.0f, 500.0f, 340.0f));

    // ---------------------------------------------------------------- header
    g.setFont (displayFont (26.0f, true, 0.4f));
    g.setColour (ink);
    g.drawText ("MH", juce::Rectangle<float> (22.0f, 8.0f, 62.0f, 40.0f),
                juce::Justification::centredLeft, false);

    g.setColour (juce::Colour (0xff2d3138));
    g.drawLine (92.0f, 16.0f, 92.0f, 48.0f, 1.0f);

    g.setFont (displayFont (17.0f, false, 3.4f));
    g.setColour (ink);
    g.drawText ("MH SIGNATURE", juce::Rectangle<float> (108.0f, 8.0f, 330.0f, 28.0f),
                juce::Justification::centredLeft, false);

    g.setFont (uiFont (9.5f, false, 3.0f));
    g.setColour (juce::Colour (0xff8d949e));
    g.drawText ("BY MILAD HASHEMI", juce::Rectangle<float> (110.0f, 34.0f, 280.0f, 14.0f),
                juce::Justification::centredLeft, false);

    g.setFont (uiFont (9.5f, false, 3.0f));
    g.setColour (juce::Colour (0xff959ca6));
    g.drawText ("SOUND WITH INTENTION", juce::Rectangle<float> (1250.0f, 26.0f, 172.0f, 14.0f),
                juce::Justification::centredRight, false);

    // ------------------------------------------------------------- signature
    const float railCx = rail.getCentreX();

    g.setFont (juce::Font (juce::Font::getDefaultSerifFontName(), 58.0f, juce::Font::italic)
                 .withExtraKerningFactor (0.01f));
    g.setColour (juce::Colour (0xfff4ede7));
    g.drawText ("Milad",
                juce::Rectangle<float> (rail.getX(), 606.0f, rail.getWidth(), 78.0f),
                juce::Justification::centred, false);

    {
        juce::Path flourish;
        flourish.startNewSubPath (railCx - 92.0f, 690.0f);
        flourish.cubicTo (railCx - 40.0f, 678.0f, railCx + 10.0f, 676.0f, railCx + 40.0f, 684.0f);
        flourish.cubicTo (railCx + 62.0f, 690.0f, railCx + 46.0f, 698.0f, railCx + 24.0f, 696.0f);
        flourish.cubicTo (railCx - 6.0f, 693.0f, railCx + 34.0f, 683.0f, railCx + 82.0f, 682.0f);

        g.setColour (accent.withAlpha (0.8f));
        g.strokePath (flourish, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
    }

    g.setFont (uiFont (10.0f, false, 3.0f));
    g.setColour (juce::Colour (0xffd3ccc6));
    g.drawText ("MUSIC / PEOPLE / EMOTION", juce::Rectangle<float> (rail.getX(), 712.0f, 364.0f, 14.0f),
                juce::Justification::centred, false);
    g.setColour (juce::Colour (0xffb9b2ac));
    g.drawText ("ALWAYS", juce::Rectangle<float> (rail.getX(), 730.0f, 364.0f, 14.0f),
                juce::Justification::centred, false);

    g.setFont (uiFont (8.5f, false, 2.6f));
    g.setColour (juce::Colour (0xff7d858f));
    g.drawText ("VERSION 1.0.0", juce::Rectangle<float> (rail.getX(), 812.0f, 200.0f, 14.0f),
                juce::Justification::centredLeft, false);

    // ---------------------------------------------------------------- footer
    g.setColour (juce::Colour (0xff191c21));
    g.drawLine (0.0f, 862.0f, (float) designW, 862.0f, 1.0f);

    g.setFont (uiFont (8.5f, false, 2.4f));
    g.setColour (juce::Colour (0xff7d858f));
    g.drawText ("dB", juce::Rectangle<float> (16.0f, 870.0f, 60.0f, 14.0f),
                juce::Justification::centredLeft, false);
    g.drawText ("dB", juce::Rectangle<float> (1348.0f, 870.0f, 60.0f, 14.0f),
                juce::Justification::centredRight, false);

    g.setFont (uiFont (10.0f, false, 3.0f));
    g.setColour (juce::Colour (0xffcfd5dc));
    g.drawText ("AN ALL-IN-ONE MIXING ENVIRONMENT",
                juce::Rectangle<float> (420.0f, 870.0f, 912.0f, 14.0f),
                juce::Justification::centred, false);

    g.setFont (uiFont (8.5f, false, 2.4f));
    g.setColour (juce::Colour (0xff7d858f));
    g.drawText ("DESIGNED BY MILAD HASHEMI",
                juce::Rectangle<float> (1080.0f, 870.0f, 260.0f, 14.0f),
                juce::Justification::centredRight, false);
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

    // open at a size that fits the screen but keeps the type at 100 %
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
