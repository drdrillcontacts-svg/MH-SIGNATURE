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
            if (v >= 1000.0f) return juce::String (v / 1000.0f, 1) + " kHz";
            return juce::String ((int) std::round (v)) + " Hz";

        case Fmt::ms:
            if (v >= 1000.0f) return juce::String (v / 1000.0f, 2) + " s";
            return juce::String ((int) std::round (v)) + " ms";
    }

    return juce::String (v, 2);
}

juce::Font labelFont (float size, float kerning)
{
    return juce::Font (juce::Font::getDefaultSansSerifFontName(), size, juce::Font::plain)
             .withExtraKerningFactor (kerning * 0.06f);
}

juce::Font numberFont (float size, float kerning)
{
    return juce::Font (juce::Font::getDefaultSansSerifFontName(), size, juce::Font::plain)
             .withExtraKerningFactor (kerning * 0.06f);
}

juce::Font titleFont (float size, float kerning)
{
    return juce::Font (juce::Font::getDefaultSansSerifFontName(), size, juce::Font::plain)
             .withExtraKerningFactor (kerning * 0.06f);
}

//==============================================================================
MhKnob::MhKnob (MHSignatureAudioProcessor& proc,
                const juce::String& paramId,
                const juce::String& labelText,
                Fmt format,
                bool bigStyle)
    : label (labelText), fmt (format), big (bigStyle)
{
    setSliderStyle (juce::Slider::RotaryVerticalDrag);
    setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    setWantsKeyboardFocus (false);
    setOpaque (false);
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    setMouseDragSensitivity (big ? 320 : 220);

    param = proc.apvts.getParameter (paramId);

    if (param != nullptr)
    {
        const auto def = param->convertFrom0to1 (param->getDefaultValue());
        setDoubleClickReturnValue (true, def);
    }

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, paramId, *this);
}

void MhKnob::mouseEnter (const juce::MouseEvent&) { hovered = true;  repaint(); }
void MhKnob::mouseExit  (const juce::MouseEvent&) { hovered = false; repaint(); }

void MhKnob::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);
    const float textH = 22.0f;

    const float d  = juce::jmin (b.getWidth() - 2.0f, b.getHeight() - textH);
    if (d <= 8.0f) return;

    const float totalH = d + textH;
    const float top = b.getY() + juce::jmax (0.0f, (b.getHeight() - totalH) * 0.5f);

    const float cx = b.getCentreX();
    const float cy = top + d * 0.5f;
    const float r  = d * 0.5f - 1.0f;

    const float startRad = juce::MathConstants<float>::pi * 1.25f;
    const float endRad   = juce::MathConstants<float>::pi * 2.75f;
    const float sweep    = endRad - startRad;

    const float norm = (float) valueToProportionOfLength (getValue());

    // ---- track -----------------------------------------------------------
    juce::Path track;
    track.addCentredArc (cx, cy, r - 1.5f, r - 1.5f, 0.0f, startRad, endRad, true);
    g.setColour (juce::Colour (0xff2a2d33));
    g.strokePath (track, juce::PathStrokeType (big ? 3.4f : 2.0f,
                                               juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    // ---- value arc -------------------------------------------------------
    if (norm > 0.002f)
    {
        juce::Path arc;
        arc.addCentredArc (cx, cy, r - 1.5f, r - 1.5f, 0.0f, startRad,
                           startRad + sweep * norm, true);

        if (glow > 0.01f)
        {
            g.setColour (accent.withAlpha (0.22f * glow));
            g.strokePath (arc, juce::PathStrokeType (big ? 12.0f : 7.0f,
                                                     juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        }

        g.setColour (hovered ? softAccent : accent);
        g.strokePath (arc, juce::PathStrokeType (big ? 3.2f : 2.2f,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    // ---- body ------------------------------------------------------------
    const float bodyR = r - (big ? 9.0f : 5.5f);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2b2e34), cx, cy - bodyR,
                                             juce::Colour (0xff08090b), cx, cy + bodyR, false));
    g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0x33ffffff), cx - bodyR * 0.35f,
                                             cy - bodyR * 0.55f, juce::Colours::transparentBlack,
                                             cx + bodyR * 0.2f, cy + bodyR * 0.4f, true));
    g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    g.setColour (juce::Colour (0x14ffffff));
    g.drawEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

    // ---- pointer ---------------------------------------------------------
    const float angle = startRad + sweep * norm;
    const float r0 = bodyR * 0.30f;
    const float r1 = bodyR * 0.86f;

    g.setColour (juce::Colour (0x66000000));
    g.drawLine (cx + std::sin (angle) * r0, cy - std::cos (angle) * r0,
                cx + std::sin (angle) * r1, cy - std::cos (angle) * r1, 2.6f);

    g.setColour (hovered ? softAccent : juce::Colour (0xffe7e2df));
    g.drawLine (cx + std::sin (angle) * r0, cy - std::cos (angle) * r0,
                cx + std::sin (angle) * r1, cy - std::cos (angle) * r1, 1.6f);

    // ---- text ------------------------------------------------------------
    const float labelY = cy + r + 2.0f;
    auto labelRect = juce::Rectangle<float> (b.getX() - 6.0f, labelY + 3.0f, b.getWidth() + 12.0f, 11.0f);
    auto valueRect = juce::Rectangle<float> (b.getX() - 6.0f, labelY + 14.0f, b.getWidth() + 12.0f, 11.0f);

    g.setFont (labelFont (big ? 9.0f : 7.4f, big ? 2.0f : 1.2f));
    g.setColour (juce::Colour (0xffd3d8de));
    g.drawText (label, labelRect, juce::Justification::centred, false);

    g.setFont (numberFont (big ? 9.0f : 7.4f, 0.4f));
    g.setColour (softAccent.withAlpha (0.95f));
    g.drawText (formatValue ((float) getValue(), fmt), valueRect, juce::Justification::centred, false);
}

//==============================================================================
MhDot::MhDot (MHSignatureAudioProcessor& proc, const juce::String& paramId, float diameter)
    : juce::Button (paramId), d (diameter)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    param = proc.apvts.getParameter (paramId);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        proc.apvts, paramId, *this);
}

void MhDot::paintButton (juce::Graphics& g, bool over, bool)
{
    const bool on = getToggleState();
    auto b = getLocalBounds().toFloat();
    const float cx = b.getCentreX(), cy = b.getCentreY();

    if (on)
    {
        g.setColour (accent.withAlpha (0.16f));
        g.fillEllipse (cx - d * 0.5f, cy - d * 0.5f, d, d);

        g.setColour (juce::Colour (0x22ff7a5f));
        g.fillEllipse (cx - d * 0.95f, cy - d * 0.95f, d * 1.9f, d * 1.9f);

        g.setColour (over ? softAccent : accent);
        g.fillEllipse (cx - d * 0.34f, cy - d * 0.34f, d * 0.68f, d * 0.68f);
    }
    else
    {
        g.setColour (juce::Colour (0xff3a3e45));
        g.drawEllipse (cx - d * 0.34f, cy - d * 0.34f, d * 0.68f, d * 0.68f, 1.2f);
    }
}

//==============================================================================
ModulePanel::ModulePanel (MHSignatureAudioProcessor& p,
                          const juce::String& title,
                          const juce::StringArray& resetIds)
    : panelTitle (title), proc (p), resetParams (resetIds)
{
    setInterceptsMouseClicks (true, true);
    setMouseCursor (juce::MouseCursor::NormalCursor);
}

juce::Rectangle<int> ModulePanel::getBodyArea() const
{
    return getLocalBounds().reduced (13, 11).withTrimmedTop (20);
}

juce::Rectangle<int> ModulePanel::getArtArea (int artHeight) const
{
    auto body = getBodyArea();
    if (artHeight <= 0) return {};
    return body.removeFromTop (artHeight);
}

juce::Rectangle<int> ModulePanel::getKnobArea (int artHeight) const
{
    auto body = getBodyArea();
    if (artHeight > 0) body.removeFromTop (artHeight + 2);
    return body;
}

void ModulePanel::setArt (int artHeight,
                          std::function<void (juce::Graphics&, juce::Rectangle<float>)> drawer)
{
    artH = artHeight;
    artDrawer = std::move (drawer);
    resized();
    repaint();
}

void ModulePanel::addKnob (const juce::String& id, const juce::String& label, Fmt fmt)
{
    auto k = std::make_unique<MhKnob> (proc, id, label, fmt);
    addAndMakeVisible (*k);
    k->setAlpha (focused ? 1.0f : 0.4f);
    k->onValueChange = [this]
    {
        if (artH > 0) repaint (getArtArea (artH));
    };
    knobs.push_back (std::move (k));
    resized();
}

void ModulePanel::addDot (const juce::String& id)
{
    dot = std::make_unique<MhDot> (proc, id, 16.0f);
    addAndMakeVisible (*dot);
    resized();
}

juce::Rectangle<int> ModulePanel::gearBounds() const
{
    return { getWidth() - 32, 8, 18, 18 };
}

void ModulePanel::setFocused (bool shouldBeFocused)
{
    focused = shouldBeFocused;

    for (auto& k : knobs) k->setAlpha (focused ? 1.0f : 0.4f);
    if (dot != nullptr) dot->setAlpha (focused ? 1.0f : 0.4f);

    repaint();
}

void ModulePanel::resized()
{
    if (dot != nullptr)
        dot->setBounds (getWidth() - 56, 9, 16, 16);

    const int n = (int) knobs.size();
    if (n == 0) return;

    auto area = getKnobArea (artH);
    if (area.getHeight() < 20) return;

    const int rows = n <= 4 ? 1 : 2;
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

void ModulePanel::mouseUp (const juce::MouseEvent& e)
{
    if (gearBounds().contains (e.getPosition()) && resetParams.size() > 0)
    {
        for (const auto& id : resetParams)
        {
            if (auto* param = proc.apvts.getParameter (id))
            {
                param->beginChangeGesture();
                param->setValueNotifyingHost (param->getDefaultValue());
                param->endChangeGesture();
            }
        }
    }
}

void ModulePanel::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);
    const float corner = 7.0f;

    // glass body
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff14171c), r.getX(), r.getY(),
                                             juce::Colour (0xff0b0d10), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, corner);

    g.setOpacity (focused ? 1.0f : 0.4f);

    g.setColour (juce::Colour (0x0effffff));
    g.fillRoundedRectangle (r.withHeight (r.getHeight() * 0.45f), corner);

    g.setColour (edgeCol);
    g.drawRoundedRectangle (r, corner, 1.0f);

    // title
    if (panelTitle.isNotEmpty())
    {
        g.setFont (labelFont (9.4f, 1.9f));
        g.setColour (juce::Colour (0xffe6eaef));
        g.drawText (panelTitle, getLocalBounds().reduced (13, 0).withTrimmedTop (9).withHeight (14),
                    centreTitle ? juce::Justification::centred
                                : juce::Justification::centredLeft, false);
    }

    if (caption.isNotEmpty())
    {
        g.setFont (labelFont (7.4f, 2.6f));
        g.setColour (juce::Colour (0xff8b939e));
        g.drawText (caption, getLocalBounds().withTrimmedTop (getHeight() - 24).withHeight (14),
                    juce::Justification::centred, false);
    }

    // reset gear
    if (resetParams.size() > 0)
    {
        auto gb = gearBounds().toFloat();
        const float gcx = gb.getCentreX(), gcy = gb.getCentreY();
        juce::Path gear;
        gear.addCentredArc (gcx, gcy, 4.6f, 4.6f, 0.0f, 0.35f, 5.5f, true);
        g.setColour (juce::Colour (gearHover ? 0xffb9c0c9 : 0xff565d66));
        g.strokePath (gear, juce::PathStrokeType (1.1f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
        for (int i = 0; i < 6; ++i)
        {
            const float a = juce::MathConstants<float>::twoPi * (float) i / 6.0f;
            g.drawLine (gcx + std::sin (a) * 5.4f, gcy - std::cos (a) * 5.4f,
                        gcx + std::sin (a) * 6.8f, gcy - std::cos (a) * 6.8f, 1.0f);
        }
    }

    if (artDrawer != nullptr && artH > 0)
        artDrawer (g, getArtArea (artH).toFloat());
}

//==============================================================================
ChoiceChip::ChoiceChip (MHSignatureAudioProcessor& p, const juce::String& paramId,
                        const juce::String& cap)
    : proc (p), caption (cap)
{
    param = proc.apvts.getParameter (paramId);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void ChoiceChip::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (juce::Colour (0xff1b1e24));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (accent.withAlpha (0.55f));
    g.drawRoundedRectangle (r, 4.0f, 1.0f);

    juce::String text = "--";

    if (auto* ch = dynamic_cast<juce::AudioParameterChoice*> (param))
        text = ch->getCurrentChoiceName().toUpperCase();

    g.setFont (labelFont (7.6f, 1.4f));
    g.setColour (juce::Colour (0xffe2e6eb));
    g.drawText (text, r.reduced (6.0f, 0.0f), juce::Justification::centredLeft, false);

    g.setFont (labelFont (7.0f, 0.6f));
    g.setColour (muted);
    g.drawText ("TYPE", r.reduced (6.0f, 0.0f), juce::Justification::centredRight, false);
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

    const float readoutH = 18.0f;
    auto readout = b.removeFromTop (readoutH);
    auto barArea = b.reduced (0.0f, 4.0f);

    const float levelDb = isInput ? proc.getInputLevelDb() : proc.getOutputLevelDb();

    // readout
    juce::String txt = levelDb <= -99.0f ? "-inf dB" : juce::String (levelDb, 1) + " dB";
    g.setFont (numberFont (8.6f, 0.6f));
    g.setColour (juce::Colour (0xffd8dce2));
    g.drawText (txt, readout, isInput ? juce::Justification::centredLeft
                                      : juce::Justification::centredRight, false);

    // scale labels sit next to the bar
    const float barW = 9.0f;
    juce::Rectangle<float> bar (isInput ? barArea.getX() + 8.0f : barArea.getRight() - 8.0f - barW,
                                barArea.getY() + 6.0f, barW, barArea.getHeight() - 26.0f);

    auto dbToY = [&] (float db)
    {
        const float n = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
        return bar.getBottom() - n * bar.getHeight();
    };

    // track
    g.setColour (juce::Colour (0xff0e1014));
    g.fillRoundedRectangle (bar.expanded (1.5f, 1.0f), 2.5f);
    g.setColour (juce::Colour (0xff1e2127));
    g.drawRoundedRectangle (bar.expanded (1.5f, 1.0f), 2.5f, 1.0f);

    // ticks + labels
    g.setFont (numberFont (6.4f, 0.4f));
    for (int db = 0; db >= -60; db -= 5)
    {
        const float y = dbToY ((float) db);
        g.setColour (juce::Colour (0x26ffffff));
        g.drawLine (isInput ? bar.getX() - 3.0f : bar.getRight(),
                    y, isInput ? bar.getX() - 1.0f : bar.getRight() + 2.0f, y, 1.0f);

        g.setColour (juce::Colour (0xff5c636d));
        auto lr = juce::Rectangle<float> (isInput ? bar.getRight() + 5.0f : bar.getX() - 27.0f,
                                          y - 5.0f, 22.0f, 10.0f);
        g.drawText (juce::String (db), lr,
                    isInput ? juce::Justification::centredLeft : juce::Justification::centredRight,
                    false);
    }

    // filled portion
    const float n = juce::jlimit (0.0f, 1.0f, (levelDb + 60.0f) / 60.0f);
    if (n > 0.001f)
    {
        juce::Rectangle<float> fill (bar.getX(), dbToY (levelDb), bar.getWidth(),
                                     bar.getBottom() - dbToY (levelDb));

        g.setGradientFill (juce::ColourGradient (accent, bar.getX(), bar.getBottom(),
                                                 softAccent, bar.getX(), bar.getY(), false));
        g.fillRoundedRectangle (fill, 2.0f);

        g.setColour (softAccent.withAlpha (0.85f));
        g.fillRect (bar.getX() - 2.0f, fill.getY() - 1.0f, bar.getWidth() + 4.0f, 1.6f);
    }
}

//==============================================================================
TabStrip::TabStrip()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setInterceptsMouseClicks (true, false);
}

juce::Rectangle<float> TabStrip::tabBounds (int i) const
{
    if (i < 0 || i >= items.size()) return {};
    const float w = (float) getWidth() / (float) items.size();
    return { (float) i * w + 2.0f, 2.0f, w - 4.0f, (float) getHeight() - 4.0f };
}

void TabStrip::setIndex (int i, bool notify)
{
    current = juce::jlimit (0, items.size() - 1, i);
    repaint();
    if (notify && onTabChanged) onTabChanged (current);
}

void TabStrip::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff0d0f13));
    g.fillRoundedRectangle (r, 16.0f);
    g.setColour (juce::Colour (0xff1c1f25));
    g.drawRoundedRectangle (r.reduced (0.5f), 16.0f, 1.0f);

    for (int i = 0; i < items.size(); ++i)
    {
        auto tb = tabBounds (i);

        if (i == current)
        {
            g.setColour (accent.withAlpha (0.18f));
            g.fillRoundedRectangle (tb.expanded (2.0f, 3.0f), 14.0f);
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe04a2a), tb.getX(), tb.getY(),
                                                     deepAccent, tb.getX(), tb.getBottom(), false));
            g.fillRoundedRectangle (tb.expanded (1.0f, 2.0f), 14.0f);
            g.setColour (juce::Colour (0xfff7ded7));
        }
        else
        {
            g.setColour (i == hovered ? juce::Colour (0xffd6dbe2) : muted);
        }

        g.setFont (labelFont (7.8f, 1.8f));
        g.drawText (items[i], tb, juce::Justification::centred, false);
    }
}

void TabStrip::mouseUp (const juce::MouseEvent& e)
{
    for (int i = 0; i < items.size(); ++i)
        if (tabBounds (i).contains (e.position))
        {
            setIndex (i);
            return;
        }
}

//==============================================================================
PresetBar::PresetBar (MHSignatureAudioProcessor& p) : proc (p)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setInterceptsMouseClicks (true, false);
}

void PresetBar::mouseMove (const juce::MouseEvent& e)
{
    const int z = e.position.x < 26.0f ? -1 : (e.position.x > (float) getWidth() - 26.0f ? 1 : 0);
    if (z != hoverZone) { hoverZone = z; repaint(); }
}

void PresetBar::mouseExit (const juce::MouseEvent&)
{
    hoverZone = 0;
    repaint();
}

void PresetBar::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff15181d), r.getX(), r.getY(),
                                             juce::Colour (0xff0c0e11), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 20.0f);
    g.setColour (juce::Colour (0xff2a2e35));
    g.drawRoundedRectangle (r, 20.0f, 1.0f);

    // chevrons
    g.setFont (labelFont (10.0f, 0.0f));
    g.setColour (hoverZone == -1 ? softAccent : juce::Colour (0xff868e99));
    g.drawText (juce::CharPointer_UTF8 ("\xe2\x80\xb9"), r.withWidth (26.0f),
                juce::Justification::centred, false);
    g.setColour (hoverZone == 1 ? softAccent : juce::Colour (0xff868e99));
    g.drawText (juce::CharPointer_UTF8 ("\xe2\x80\xba"), r.withTrimmedLeft (r.getWidth() - 26.0f),
                juce::Justification::centred, false);

    // text block
    auto txt = r.reduced (26.0f, 7.0f);

    juce::String cat = MHSignatureAudioProcessor::getFactoryPresetCategory (proc.getCurrentPresetIndex());
    const int idx = proc.getCurrentPresetIndex() + 1;
    juce::String top = cat + " Preset \xc2\xb7 " + juce::String (idx).paddedLeft ('0', 3);
    if (proc.isPresetEdited()) top += "  *";

    g.setFont (numberFont (7.4f, 1.2f));
    g.setColour (juce::Colour (0xff8f97a2));
    g.drawText (top.toUpperCase(), txt.removeFromTop (txt.getHeight() * 0.45f),
                juce::Justification::centred, false);

    g.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 12.5f, juce::Font::plain)
                 .withExtraKerningFactor (0.02f));
    g.setColour (ink);
    g.drawText (proc.getCurrentPresetName().fromFirstOccurrenceOf (" ", false, false),
                txt, juce::Justification::centred, false);
}

void PresetBar::mouseUp (const juce::MouseEvent& e)
{
    if (e.position.x < 26.0f)       { proc.prevPreset(); repaint(); }
    else if (e.position.x > (float) getWidth() - 26.0f) { proc.nextPreset(); repaint(); }
    else                            { showBrowser(); }
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
namespace
{
    float pv (MHSignatureAudioProcessor& p, const char* id)
    {
        if (auto* a = p.apvts.getRawParameterValue (id))
            return a->load();
        return 0.0f;
    }

}

//==============================================================================
juce::Path makeSignaturePath()
{
    // a single flourish stroke that sits under the wordmark
    juce::Path p;
    p.startNewSubPath (6.0f, 18.0f);
    p.cubicTo (60.0f, 2.0f, 120.0f, 2.0f, 150.0f, 12.0f);
    p.cubicTo (176.0f, 21.0f, 150.0f, 30.0f, 128.0f, 26.0f);
    p.cubicTo (100.0f, 21.0f, 140.0f, 8.0f, 184.0f, 6.0f);
    return p;
}

//==============================================================================
class SpectrumPanel : public ModulePanel
{
public:
    explicit SpectrumPanel (MHSignatureAudioProcessor& p)
        : ModulePanel (p, juce::String(), juce::StringArray()), proc (p)
    {
        setMouseCursor (juce::MouseCursor::NormalCursor);
    }

    void tick()
    {
        proc.getSpectrum (spec.data(), false);
        proc.getSpectrum (specPre.data(), true);
        proc.getWaveform (wave.data());

        loudAccum = juce::jmax (loudAccum * 0.92f, proc.getOutputLevelDb());
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

        auto body = getBodyArea();

        // ---- tabs --------------------------------------------------------
        for (int i = 0; i < 3; ++i)
        {
            auto tb = tabRect (i);
            if (i == mode)
            {
                g.setColour (accent.withAlpha (0.16f));
                g.fillRoundedRectangle (tb.expanded (3.0f, 2.0f), 9.0f);
                g.setColour (accent);
                g.setFont (labelFont (7.2f, 1.5f));
            }
            else
            {
                g.setColour (juce::Colour (0xff7d858f));
                g.setFont (labelFont (7.2f, 1.5f));
            }
            g.drawText (tabNames[i], tb, juce::Justification::centred, false);
        }

        auto chip = chipRect();
        g.setFont (labelFont (6.8f, 1.3f));
        g.setColour (pre ? accent : juce::Colour (0xff6f7781));
        g.drawText ("PRE/POST", chip, juce::Justification::centredRight, false);

        // ---- graph -------------------------------------------------------
        auto graph = body.withTrimmedTop (26);
        auto inner = graph.withTrimmedLeft (20).withTrimmedBottom (16);

        g.setColour (juce::Colour (0xff0a0c10));
        g.fillRoundedRectangle (graph.toFloat(), 4.0f);

        auto dbToY = [&] (float db)
        {
            const float n = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
            return inner.getBottom() - n * inner.getHeight();
        };
        auto freqToX = [&] (float f)
        {
            const float t = juce::jlimit (0.0f, 1.0f,
                                          std::log (juce::jmax (20.0f, f) / 20.0f) / std::log (1000.0f));
            return inner.getX() + t * inner.getWidth();
        };

        g.setFont (numberFont (5.8f, 0.3f));
        for (int db = 0; db >= -60; db -= 10)
        {
            const float y = dbToY ((float) db);
            g.setColour (gridCol);
            g.drawLine (inner.getX(), y, inner.getRight(), y, 1.0f);
            g.setColour (juce::Colour (0xff5f6771));
            g.drawText (juce::String (db), juce::Rectangle<float> (graph.getX() + 2.0f, y - 5.0f,
                                                                  16.0f, 10.0f),
                        juce::Justification::centredRight, false);
        }

        const float freqs[] { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f,
                              5000.0f, 10000.0f, 20000.0f };
        const char* labels[] { "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k" };

        for (int i = 0; i < 10; ++i)
        {
            const float x = freqToX (freqs[i]);
            g.setColour (gridCol);
            g.drawLine (x, inner.getY(), x, inner.getBottom(), 1.0f);
            g.setColour (juce::Colour (0xff5f6771));
            g.setFont (numberFont (5.6f, 0.2f));
            g.drawText (labels[i], juce::Rectangle<float> (x - 12.0f, inner.getBottom() + 2.0f,
                                                           24.0f, 10.0f),
                        juce::Justification::centred, false);
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

            g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.55f), 0.0f, inner.getY(),
                                                     accent.withAlpha (0.03f), 0.0f, inner.getBottom(), false));
            g.fillPath (fill);

            g.setColour (softAccent.withAlpha (0.95f));
            g.strokePath (line, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved,
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
            g.strokePath (p, juce::PathStrokeType (1.1f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
            g.setColour (juce::Colour (0x1effffff));
            g.drawLine (inner.getX(), inner.getCentreY(), inner.getRight(), inner.getCentreY(), 1.0f);
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
            g.strokePath (p, juce::PathStrokeType (1.2f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
        }

        g.setColour (juce::Colour (0xff1b1e24));
        g.drawRoundedRectangle (graph.toFloat(), 4.0f, 1.0f);
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        for (int i = 0; i < 3; ++i)
            if (tabRect (i).expanded (3.0f, 2.0f).contains (e.position))
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

        ModulePanel::mouseUp (e);
    }

private:
    juce::Rectangle<float> tabRect (int i) const
    {
        const float widths[] { 66.0f, 74.0f, 66.0f };
        float x = (float) getBodyArea().getX();
        for (int k = 0; k < i; ++k) x += widths[k];
        return { x, (float) getBodyArea().getY() + 1.0f, widths[i], 18.0f };
    }

    juce::Rectangle<float> chipRect() const
    {
        auto body = getBodyArea();
        return { (float) body.getRight() - 52.0f, (float) body.getY() + 1.0f, 52.0f, 18.0f };
    }

    MHSignatureAudioProcessor& proc;
    int mode = 0;
    bool pre = false;

    juce::StringArray tabNames { "SPECTRUM", "WAVEFORM", "LOUDNESS" };

    std::array<float, MHSignatureAudioProcessor::numSpectrumBands> spec {}, specPre {};
    std::array<float, MHSignatureAudioProcessor::numScopeSamples> wave {};
    std::array<float, 128> hist {};
    int histIndex = 0;
    int loudCounter = 0;
    float loudAccum = -90.0f;
};

//==============================================================================
Canvas::Canvas (MHSignatureAudioProcessor& p) : proc (p)
{
    setOpaque (true);

    portrait = juce::ImageFileFormat::loadFrom (BinaryData::portrait_jpg,
                                                (size_t) BinaryData::portrait_jpgSize);
    signature = makeSignaturePath();

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

    // ---- signature engine ------------------------------------------------
    enginePanel = std::make_unique<ModulePanel> (proc, "SIGNATURE ENGINE", juce::StringArray());
    enginePanel->centreTitle = true;
    enginePanel->caption = "TURN IDEAS INTO SOUND";
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

        engineMacros[0]->setBounds (34, 48, 80, 88);
        engineMacros[1]->setBounds (350, 48, 80, 88);
        engineMacros[2]->setBounds (34, 136, 80, 88);
        engineMacros[3]->setBounds (350, 136, 80, 88);

        engineKnob = std::make_unique<MhKnob> (proc, Params::intensity, "INTENSITY", Fmt::pct0, true);
        enginePanel->addAndMakeVisible (*engineKnob);
        engineKnob->setBounds (150, 40, 164, 186);
    }

    // ---- EQ --------------------------------------------------------------
    eqPanel = std::make_unique<ModulePanel> (proc, "EQ",
        juce::StringArray { Params::eqHpf, Params::eqLowF, Params::eqLowG, Params::eqLmF,
                            Params::eqLmG, Params::eqHmF, Params::eqHmG, Params::eqHighF,
                            Params::eqHighG, Params::eqTilt });
    eqPanel->addDot (Params::eqOn);
    eqPanel->addKnob (Params::eqLowG,   "LOW",   Fmt::db1);
    eqPanel->addKnob (Params::eqLmG,    "LOMID", Fmt::db1);
    eqPanel->addKnob (Params::eqHmG,    "HIMID", Fmt::db1);
    eqPanel->addKnob (Params::eqHighG,  "HIGH",  Fmt::db1);
    eqPanel->addKnob (Params::eqTilt,   "TILT",  Fmt::db1);
    eqPanel->addKnob (Params::eqHpf,    "HPF",   Fmt::hz);
    eqPanel->setArt (54, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float lowG  = pv (proc, Params::eqLowG);
        const float lmG   = pv (proc, Params::eqLmG);
        const float hmG   = pv (proc, Params::eqHmG);
        const float highG = pv (proc, Params::eqHighG);
        const float tilt  = pv (proc, Params::eqTilt);
        const float hpf   = pv (proc, Params::eqHpf);

        g.setColour (juce::Colour (0x1affffff));
        for (int i = 1; i < 4; ++i)
            g.drawLine (a.getX() + a.getWidth() * (float) i * 0.25f, a.getY(),
                        a.getX() + a.getWidth() * (float) i * 0.25f, a.getBottom(), 1.0f);
        g.setColour (juce::Colour (0x22ffffff));
        g.drawLine (a.getX(), a.getCentreY(), a.getRight(), a.getCentreY(), 1.0f);

        juce::Path curve, fill;
        const int n = 110;

        for (int i = 0; i < n; ++i)
        {
            const float t = (float) i / (float) (n - 1);
            const float f = 20.0f * std::pow (1000.0f, t);

            auto bell = [] (float f, float fc, float oct)
            {
                const float d = std::log2 (juce::jmax (1.0f, f) / fc) / oct;
                return std::exp (-0.5f * d * d);
            };

            float db = lowG * bell (f, 65.0f, 1.15f)
                     + lmG  * bell (f, 152.0f, 0.85f)
                     + hmG  * bell (f, 3500.0f, 0.95f)
                     + highG * bell (f, 9000.0f, 1.5f)
                     + tilt * (t - 0.5f) * 6.0f;

            if (f < hpf)
                db -= 10.0f * std::log2 (hpf / juce::jmax (20.0f, f));

            const float y = a.getBottom() - juce::jlimit (0.0f, 1.0f, (db + 14.0f) / 28.0f) * a.getHeight();
            const float x = a.getX() + t * a.getWidth();

            if (i == 0) { curve.startNewSubPath (x, y); fill.startNewSubPath (x, a.getBottom()); }
            curve.lineTo (x, y);
            fill.lineTo (x, y);
        }

        fill.lineTo (a.getRight(), a.getBottom());
        fill.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.30f), 0.0f, a.getY(),
                                                 accent.withAlpha (0.02f), 0.0f, a.getBottom(), false));
        g.fillPath (fill);

        g.setColour (softAccent.withAlpha (0.9f));
        g.strokePath (curve, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    });
    addAndMakeVisible (*eqPanel);

    // ---- COMPRESSOR ------------------------------------------------------
    compPanel = std::make_unique<ModulePanel> (proc, "COMPRESSOR",
        juce::StringArray { Params::dynThr, Params::dynRatio, Params::dynAtt, Params::dynRel,
                            Params::dynMakeup, Params::dynMix, Params::limCeiling });
    compPanel->addDot (Params::dynOn);
    compPanel->addKnob (Params::dynThr,    "THRESHOLD", Fmt::db1);
    compPanel->addKnob (Params::dynRatio,  "RATIO",     Fmt::ratio);
    compPanel->addKnob (Params::dynAtt,    "ATTACK",    Fmt::ms);
    compPanel->addKnob (Params::dynRel,    "RELEASE",   Fmt::ms);
    compPanel->addKnob (Params::dynMakeup, "MAKEUP",    Fmt::db1);
    compPanel->addKnob (Params::dynMix,    "MIX",       Fmt::pct0);
    compPanel->setArt (54, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float gr = proc.getGainReductionDb();
        auto left = a.withWidth (a.getWidth() * 0.30f).reduced (0.0f, 4.0f);
        auto right = a.withTrimmedLeft (a.getWidth() * 0.36f).reduced (0.0f, 4.0f);

        // transfer curve
        g.setColour (juce::Colour (0xff1d2026));
        g.drawRect (left, 1.0f);

        juce::Path io;
        const float thr = pv (proc, Params::dynThr);
        const float ratio = juce::jmax (1.0f, pv (proc, Params::dynRatio));
        for (int i = 0; i <= 40; ++i)
        {
            const float t = (float) i / 40.0f;
            const float in = -60.0f + 60.0f * t;
            const float out = in <= thr ? in : thr + (in - thr) / ratio;
            const float x = left.getX() + t * left.getWidth();
            const float y = left.getBottom() - juce::jlimit (0.0f, 1.0f, (out + 60.0f) / 60.0f) * left.getHeight();
            if (i == 0) io.startNewSubPath (x, y); else io.lineTo (x, y);
        }
        g.setColour (accent.withAlpha (0.85f));
        g.strokePath (io, juce::PathStrokeType (1.2f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

        // gain reduction bar
        g.setFont (labelFont (6.4f, 1.4f));
        g.setColour (juce::Colour (0xff7f8792));
        g.drawText ("GAIN-REDUCTION", right.withHeight (9.0f), juce::Justification::centredLeft, false);

        auto bar = right.withTrimmedTop (12.0f);
        g.setColour (juce::Colour (0xff0d0f13));
        g.fillRoundedRectangle (bar, 2.0f);

        const float amount = juce::jlimit (0.0f, 1.0f, -gr / 12.0f);
        if (amount > 0.002f)
        {
            g.setGradientFill (juce::ColourGradient (accent, bar.getX(), bar.getCentreY(),
                                                     softAccent, bar.getRight(), bar.getCentreY(), false));
            g.fillRoundedRectangle (bar.withWidth (juce::jmax (3.0f, bar.getWidth() * amount)), 2.0f);
        }

        juce::String grTxt = gr < -0.05f ? juce::String (gr, 1) + " dB" : "0.0 dB";
        g.setFont (numberFont (8.0f, 0.6f));
        g.setColour (ink);
        g.drawText (grTxt, bar, juce::Justification::centredRight, false);
    });
    addAndMakeVisible (*compPanel);

    // ---- SATURATION / COLOUR --------------------------------------------
    satPanel = std::make_unique<ModulePanel> (proc, "SATURATION",
        juce::StringArray { Params::satDrive, Params::satTone, Params::satHf, Params::satMix });
    satPanel->addDot (Params::satOn);
    satPanel->addKnob (Params::satDrive, "DRIVE", Fmt::db1);
    satPanel->addKnob (Params::satTone,  "TONE",  Fmt::pct0);
    satPanel->addKnob (Params::satMix,   "MIX",   Fmt::pct0);
    satPanel->setArt (72, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float drive = juce::jlimit (0.0f, 1.0f, pv (proc, Params::satDrive) / 18.0f);
        a = a.withTrimmedTop (16.0f);
        auto waveArea = a.withWidth (a.getWidth() * 0.58f);

        juce::Path p;
        const int n = 120;
        for (int i = 0; i < n; ++i)
        {
            const float t = (float) i / (float) (n - 1);
            const float x = waveArea.getX() + t * waveArea.getWidth();
            const float s = std::sin (t * juce::MathConstants<float>::twoPi * 3.0f);
            const float y = waveArea.getCentreY() - std::tanh (s * (1.0f + drive * 6.0f))
                                                 * waveArea.getHeight() * 0.40f;
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }

        g.setColour (softAccent.withAlpha (0.92f));
        g.strokePath (p, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

        // harmonic bars
        auto bars = a.withTrimmedLeft (a.getWidth() * 0.62f).withTrimmedRight (4.0f);
        const float heights[] { 1.0f, 0.42f + drive * 0.35f, 0.18f + drive * 0.3f };
        g.setFont (numberFont (5.6f, 0.2f));

        for (int i = 0; i < 3; ++i)
        {
            auto br = bars.withWidth (bars.getWidth() / 3.0f).withTrimmedLeft ((float) i * bars.getWidth() / 3.0f);
            br = br.reduced (3.0f, 0.0f);
            auto full = br.withTrimmedTop (10.0f).withTrimmedBottom (10.0f);
            g.setColour (juce::Colour (0xff13161a));
            g.fillRect (full);
            g.setGradientFill (juce::ColourGradient (accent, full.getX(), full.getBottom(),
                                                     deepAccent, full.getX(), full.getY(), false));
            g.fillRect (full.withTop (full.getBottom() - full.getHeight() * juce::jlimit (0.1f, 1.0f, heights[i])));
            g.setColour (juce::Colour (0xff6d7580));
            g.drawText (juce::String (i + 1), br.withHeight (8.0f), juce::Justification::centred, false);
        }


    });
    addAndMakeVisible (*satPanel);

    satType = std::make_unique<ChoiceChip> (proc, Params::satType, "TYPE");
    satPanel->addAndMakeVisible (*satType);

    // ---- REVERB ---------------------------------------------------------
    revPanel = std::make_unique<ModulePanel> (proc, "REVERB",
        juce::StringArray { Params::revSize, Params::revDecay, Params::revDamp,
                            Params::revPre, Params::revMix });
    revPanel->addDot (Params::revOn);
    revPanel->addKnob (Params::revDecay, "DECAY",   Fmt::sec1);
    revPanel->addKnob (Params::revPre,   "PRE-DLY", Fmt::ms);
    revPanel->addKnob (Params::revDamp,  "DAMP",    Fmt::pct0);
    revPanel->addKnob (Params::revMix,   "MIX",     Fmt::pct0);
    revPanel->setArt (66, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float decay = juce::jlimit (0.3f, 12.0f, pv (proc, Params::revDecay));
        const float damp = juce::jlimit (0.0f, 1.0f, pv (proc, Params::revDamp) * 0.01f);
        const float mix = juce::jlimit (0.0f, 1.0f, pv (proc, Params::revMix) * 0.01f);

        juce::Path p, fill;
        const int n = 90;
        for (int i = 0; i < n; ++i)
        {
            const float t = (float) i / (float) (n - 1);
            const float e = std::exp (-t * (6.0f / decay) * (1.0f + damp * 1.4f));
            const float x = a.getX() + t * a.getWidth();
            const float y = a.getBottom() - e * a.getHeight() * (0.35f + 0.6f * mix);
            if (i == 0) { p.startNewSubPath (x, y); fill.startNewSubPath (x, a.getBottom()); }
            p.lineTo (x, y);
            fill.lineTo (x, y);
        }
        fill.lineTo (a.getRight(), a.getBottom());
        fill.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.32f), 0.0f, a.getY(),
                                                 accent.withAlpha (0.01f), 0.0f, a.getBottom(), false));
        g.fillPath (fill);
        g.setColour (softAccent.withAlpha (0.9f));
        g.strokePath (p, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    });
    addAndMakeVisible (*revPanel);

    // ---- DELAY ----------------------------------------------------------
    dlyPanel = std::make_unique<ModulePanel> (proc, "DELAY",
        juce::StringArray { Params::dlyTime, Params::dlyFb, Params::dlyFilter, Params::dlyMix });
    dlyPanel->addDot (Params::dlyOn);
    dlyPanel->addKnob (Params::dlyTime,   "TIME",     Fmt::ms);
    dlyPanel->addKnob (Params::dlyFb,     "FEEDBACK", Fmt::pct0);
    dlyPanel->addKnob (Params::dlyFilter, "FILTER",   Fmt::hz);
    dlyPanel->addKnob (Params::dlyMix,    "MIX",      Fmt::pct0);
    dlyPanel->setArt (66, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float fb = juce::jlimit (0.0f, 1.0f, pv (proc, Params::dlyFb) * 0.01f);
        const int cols = 11, rows = 4;
        const float dx = a.getWidth() / (float) (cols - 1);
        const float dy = a.getHeight() / (float) (rows + 1);

        for (int c = 0; c < cols; ++c)
        {
            const float amp = std::pow (juce::jmax (0.0f, fb), (float) c * 0.55f);
            for (int r = 0; r < rows; ++r)
            {
                const bool alt = ((c + r) % 2) == 0;
                const float alpha = juce::jlimit (0.06f, 1.0f, amp * (alt ? 1.0f : 0.55f));
                const float d = 2.0f + 1.4f * amp;
                g.setColour ((alt ? accent : deepAccent.withAlpha (0.9f)).withAlpha (alpha * 0.9f));
                g.fillEllipse (a.getX() + (float) c * dx - d * 0.5f,
                               a.getY() + dy * (float) (r + 1) - d * 0.5f, d, d);
            }
        }
    });
    addAndMakeVisible (*dlyPanel);

    // ---- MODULATION -----------------------------------------------------
    modPanel = std::make_unique<ModulePanel> (proc, "MODULATION",
        juce::StringArray { Params::modRate, Params::modDepth, Params::modMix });
    modPanel->addDot (Params::modOn);
    modPanel->addKnob (Params::modRate,  "RATE",  Fmt::raw1);
    modPanel->addKnob (Params::modDepth, "DEPTH", Fmt::pct0);
    modPanel->addKnob (Params::modMix,   "MIX",   Fmt::pct0);
    modPanel->setArt (66, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float depth = juce::jlimit (0.0f, 1.0f, pv (proc, Params::modDepth) * 0.01f);
        const float mix = juce::jlimit (0.0f, 1.0f, pv (proc, Params::modMix) * 0.01f);

        auto waveY = [&] (float t)
        {
            return a.getCentreY() - std::sin (t * juce::MathConstants<float>::twoPi
                                              + lfoPhase * juce::MathConstants<float>::twoPi)
                                     * a.getHeight() * 0.42f * (0.15f + 0.85f * depth);
        };

        juce::Path p;
        const int n = 90;
        for (int i = 0; i < n; ++i)
        {
            const float t = (float) i / (float) (n - 1);
            const float x = a.getX() + t * a.getWidth();
            if (i == 0) p.startNewSubPath (x, waveY (t)); else p.lineTo (x, waveY (t));
        }

        g.setColour (softAccent.withAlpha (0.55f + 0.45f * mix));
        g.strokePath (p, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

        g.setColour (juce::Colour (0x16ffffff));
        g.drawLine (a.getX(), a.getCentreY(), a.getRight(), a.getCentreY(), 1.0f);
    });
    addAndMakeVisible (*modPanel);

    // ---- STEREO IMAGER --------------------------------------------------
    imgPanel = std::make_unique<ModulePanel> (proc, "STEREO IMAGER",
        juce::StringArray { Params::imgWidth, Params::imgWLow, Params::imgWMid,
                            Params::imgWHigh, Params::imgWAir, Params::imgBalance });
    imgPanel->addDot (Params::imgOn);
    imgPanel->addKnob (Params::imgWidth,   "WIDTH",   Fmt::pct0);
    imgPanel->addKnob (Params::imgBalance, "BALANCE", Fmt::int0);
    imgPanel->setArt (72, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float width = juce::jlimit (0.0f, 2.0f, pv (proc, Params::imgWidth) * 0.01f);
        const float bal = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::imgBalance) * 0.01f);

        auto centre = juce::Point<float> (a.getCentreX(), a.getBottom() - 4.0f);
        const float r = juce::jmin (a.getWidth() * 0.42f, a.getHeight() - 8.0f);

        juce::Path arc;
        arc.addCentredArc (centre.x, centre.y, r, r, 0.0f, juce::MathConstants<float>::pi,
                           juce::MathConstants<float>::twoPi, true);
        g.setColour (juce::Colour (0xff272b31));
        g.strokePath (arc, juce::PathStrokeType (1.2f));

        // the "petal": energy spread between the channels
        const float spread = juce::jlimit (0.05f, 1.0f, width * 0.5f) * (juce::MathConstants<float>::halfPi);
        const float rot = bal * 0.5f;

        juce::Path petal;
        petal.startNewSubPath (centre.x, centre.y);
        petal.addCentredArc (centre.x, centre.y, r * 0.95f, r * 0.95f, 0.0f,
                             -juce::MathConstants<float>::halfPi - rot - spread * 0.5f,
                             -juce::MathConstants<float>::halfPi - rot + spread * 0.5f, true);
        petal.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.65f), centre.x, centre.y,
                                                 deepAccent.withAlpha (0.05f),
                                                 centre.x, centre.y - r, true));
        g.fillPath (petal);

        g.setColour (accent.withAlpha (0.55f));
        g.drawLine (centre.x, centre.y, centre.x + std::sin (rot) * r * 0.95f,
                    centre.y - std::cos (rot) * r * 0.95f, 1.2f);

        g.setColour (juce::Colour (0xff3a3f46));
        g.drawLine (centre.x, centre.y, centre.x - r, centre.y, 1.0f);
        g.drawLine (centre.x, centre.y, centre.x + r, centre.y, 1.0f);
    });
    addAndMakeVisible (*imgPanel);

    // ---- VOCAL CONTROL --------------------------------------------------
    vocPanel = std::make_unique<ModulePanel> (proc, "VOCAL CONTROL",
        juce::StringArray { Params::vocHpf, Params::vocBodyG, Params::vocThr, Params::vocRatio,
                            Params::vocDeessF, Params::vocDeessR, Params::vocPresG,
                            Params::vocAirG, Params::vocSendRev });
    vocPanel->addDot (Params::vocOn);
    vocPanel->addKnob (Params::vocHpf,    "HPF",     Fmt::hz);
    vocPanel->addKnob (Params::vocBodyG,  "BODY",    Fmt::db1);
    vocPanel->addKnob (Params::vocThr,    "THRESH",  Fmt::db1);
    vocPanel->addKnob (Params::vocRatio,  "RATIO",   Fmt::ratio);
    vocPanel->addKnob (Params::vocDeessF, "DE-ESS",  Fmt::hz);
    vocPanel->addKnob (Params::vocDeessR, "SIBIL",   Fmt::db1);
    vocPanel->addKnob (Params::vocPresG,  "PRESENCE",Fmt::db1);
    vocPanel->addKnob (Params::vocAirG,   "AIR",     Fmt::db1);
    addAndMakeVisible (*vocPanel);

    // ---- TRANSIENT SHAPER -----------------------------------------------
    trnPanel = std::make_unique<ModulePanel> (proc, "TRANSIENT SHAPER",
        juce::StringArray { Params::trnAtt, Params::trnSus, Params::trnFreq });
    trnPanel->addDot (Params::trnOn);
    trnPanel->addKnob (Params::trnAtt,  "ATTACK",  Fmt::int0);
    trnPanel->addKnob (Params::trnSus,  "SUSTAIN", Fmt::int0);
    trnPanel->addKnob (Params::trnFreq, "SPLIT",   Fmt::hz);
    trnPanel->setArt (46, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float att = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::trnAtt) * 0.01f);
        const float sus = juce::jlimit (-1.0f, 1.0f, pv (proc, Params::trnSus) * 0.01f);

        juce::Path p, fill;
        const int n = 100;
        for (int i = 0; i < n; ++i)
        {
            const float t = (float) i / (float) (n - 1);
            const float env = std::exp (-t * 4.0f) * (0.55f + 0.45f * att)
                            + (1.0f - std::exp (-t * 10.0f)) * 0.25f * (1.0f + sus);
            const float x = a.getX() + t * a.getWidth();
            const float y = a.getBottom() - juce::jlimit (0.0f, 1.0f, env) * a.getHeight() * 0.92f;
            if (i == 0) { p.startNewSubPath (x, y); fill.startNewSubPath (x, a.getBottom()); }
            p.lineTo (x, y);
            fill.lineTo (x, y);
        }
        fill.lineTo (a.getRight(), a.getBottom());
        fill.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.28f), 0.0f, a.getY(),
                                                 accent.withAlpha (0.01f), 0.0f, a.getBottom(), false));
        g.fillPath (fill);
        g.setColour (softAccent.withAlpha (0.9f));
        g.strokePath (p, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    });
    addAndMakeVisible (*trnPanel);

    // ---- UTILITY / I-O --------------------------------------------------
    utilPanel = std::make_unique<ModulePanel> (proc, "I/O",
        juce::StringArray { Params::ioInput, Params::ioOutput, Params::ioMix });
    utilPanel->addKnob (Params::ioInput,  "IN",  Fmt::db1);
    utilPanel->addKnob (Params::ioOutput, "OUT", Fmt::db1);
    utilPanel->addKnob (Params::ioMix,    "MIX", Fmt::pct0);
    utilPanel->setArt (30, [this] (juce::Graphics& g, juce::Rectangle<float> a)
    {
        g.setFont (labelFont (6.2f, 1.2f));
        g.setColour (juce::Colour (0xff7c848e));
        g.drawText ("MONO", a.withTrimmedLeft (20.0f).withWidth (40.0f),
                    juce::Justification::centredLeft, false);
        g.drawText ("BYP", a.withTrimmedLeft (86.0f).withWidth (34.0f),
                    juce::Justification::centredLeft, false);
    });
    addAndMakeVisible (*utilPanel);

    monoDot = std::make_unique<MhDot> (proc, Params::ioMono, 12.0f);
    bypassDot = std::make_unique<MhDot> (proc, Params::ioBypass, 12.0f);
    utilPanel->addAndMakeVisible (*monoDot);
    utilPanel->addAndMakeVisible (*bypassDot);

    {
        auto art = utilPanel->getArtArea (30);
        monoDot->setBounds (art.getX(), art.getCentreY() - 7, 14, 14);
        bypassDot->setBounds (art.getX() + 66, art.getCentreY() - 7, 14, 14);
        const auto satArt = satPanel->getArtArea (72);
        satType->setBounds (satArt.getRight() - 100, satArt.getY(), 100, 16);
    }

    buildLayout();
    setSize (designW, designH);
    startTimerHz (24);
}

Canvas::~Canvas() = default;

//==============================================================================
void Canvas::buildLayout()
{
    inMeter->setBounds  (14, 86, 54, 812);
    outMeter->setBounds (1308, 86, 54, 812);

    tabs->setBounds (452, 20, 468, 32);
    presetBar->setBounds (932, 16, 240, 40);

    enginePanel->setBounds   (448, 74, 464, 278);
    spectrumPanel->setBounds (920, 74, 380, 278);

    eqPanel->setBounds   (448, 360, 268, 184);
    compPanel->setBounds (724, 360, 308, 184);
    satPanel->setBounds  (1040, 360, 260, 184);

    revPanel->setBounds (448, 552, 192, 214);
    dlyPanel->setBounds (648, 552, 252, 214);
    modPanel->setBounds (908, 552, 192, 214);
    imgPanel->setBounds (1108, 552, 190, 214);

    utilPanel->setBounds (448, 774, 150, 160);
    vocPanel->setBounds  (606, 774, 330, 160);
    trnPanel->setBounds  (944, 774, 354, 160);

    applyFocus (tabs->getIndex());
}

void Canvas::resized()
{
    // the canvas lives in a fixed design space; the editor scales the whole thing
}

void Canvas::applyFocus (int tab)
{
    eqPanel->setFocused   (tab == 0 || tab == 1);
    compPanel->setFocused (tab == 0 || tab == 2);
    trnPanel->setFocused  (tab == 0 || tab == 2);
    vocPanel->setFocused  (tab == 0 || tab == 2);
    satPanel->setFocused  (tab == 0 || tab == 3);
    revPanel->setFocused  (tab == 0 || tab == 4);
    dlyPanel->setFocused  (tab == 0 || tab == 4);
    modPanel->setFocused  (tab == 0 || tab == 5);
    imgPanel->setFocused  (tab == 0 || tab == 6);
}

void Canvas::timerCallback()
{
    lfoPhase += 0.0065f;
    if (lfoPhase > 1.0f) lfoPhase -= 1.0f;

    engineGlow = juce::jmin (1.0f, 0.15f + proc.getEngineActivity() * 1.4f);
    if (engineKnob != nullptr) engineKnob->setAccentGlow (engineGlow);

    inMeter->repaint();
    outMeter->repaint();
    spectrumPanel->tick();

    compPanel->repaint (compPanel->getArtArea (54));
    modPanel->repaint (modPanel->getArtArea (66));
    imgPanel->repaint (imgPanel->getArtArea (72));
    dlyPanel->repaint (dlyPanel->getArtArea (66));

    presetBar->repaint();
}

//==============================================================================
void Canvas::paint (juce::Graphics& g)
{
    g.fillAll (noir);

    // ------------------------------------------------------------- portrait
    const juce::Rectangle<float> portraitArea (0.0f, 0.0f, 440.0f, (float) designH);

    if (portrait.isValid())
    {
        const float iw = (float) portrait.getWidth();
        const float ih = (float) portrait.getHeight();
        const float s = juce::jmax (portraitArea.getWidth() / iw, portraitArea.getHeight() / ih);
        const float w = iw * s, h = ih * s;
        const float x = portraitArea.getCentreX() - w * 0.5f;
        const float y = portraitArea.getY() + 40.0f;

        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (portraitArea.toNearestInt());
        g.drawImage (portrait, juce::Rectangle<float> (x, y, w, h),
                     juce::RectanglePlacement::stretchToFit, false);
    }
    else
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a1108), 0.0f, 0.0f,
                                                 noir, 0.0f, (float) designH, false));
        g.fillRect (portraitArea);
    }

    // fades so the column melts into the interface
    {
        juce::ColourGradient bottom (juce::Colours::transparentBlack, 0.0f, 620.0f,
                                     noir.withAlpha (0.96f), 0.0f, 858.0f, false);
        g.setGradientFill (bottom);
        g.fillRect (juce::Rectangle<float> (0.0f, 620.0f, 440.0f, 320.0f));

        juce::ColourGradient rightEdge (juce::Colours::transparentBlack, 340.0f, 0.0f,
                                        noir, 440.0f, 0.0f, false);
        g.setGradientFill (rightEdge);
        g.fillRect (juce::Rectangle<float> (340.0f, 0.0f, 100.0f, 900.0f));

        g.setGradientFill (juce::ColourGradient (noir, 0.0f, 0.0f,
                                                 juce::Colours::transparentBlack, 0.0f, 128.0f, false));
        g.fillRect (juce::Rectangle<float> (0.0f, 0.0f, 440.0f, 128.0f));

        juce::ColourGradient leftEdge (noir.withAlpha (0.9f), 0.0f, 0.0f,
                                       juce::Colours::transparentBlack, 70.0f, 0.0f, false);
        g.setGradientFill (leftEdge);
        g.fillRect (juce::Rectangle<float> (0.0f, 0.0f, 70.0f, 900.0f));
    }

    // vermilion bloom behind the engine panel
    {
        juce::ColourGradient bloom (accent.withAlpha (0.07f), 440.0f, 74.0f,
                                    juce::Colours::transparentBlack, 780.0f, 300.0f, true);
        g.setGradientFill (bloom);
        g.fillRect (juce::Rectangle<float> (430.0f, 60.0f, 480.0f, 320.0f));
    }

    // ---------------------------------------------------------------- header
    g.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 27.0f, juce::Font::plain)
                 .withExtraKerningFactor (0.02f));
    g.setColour (ink);
    g.drawText ("MH", juce::Rectangle<float> (20.0f, 12.0f, 60.0f, 40.0f),
                juce::Justification::centredLeft, false);

    g.setColour (juce::Colour (0xff2d3138));
    g.drawLine (86.0f, 18.0f, 86.0f, 50.0f, 1.0f);

    g.setFont (titleFont (16.0f, 3.4f));
    g.setColour (ink);
    g.drawText ("MH SIGNATURE", juce::Rectangle<float> (102.0f, 10.0f, 330.0f, 26.0f),
                juce::Justification::centredLeft, false);

    g.setFont (labelFont (7.2f, 2.8f));
    g.setColour (juce::Colour (0xff8d949e));
    g.drawText ("BY MILAD HASHEMI", juce::Rectangle<float> (104.0f, 34.0f, 260.0f, 14.0f),
                juce::Justification::centredLeft, false);

    g.setFont (labelFont (7.4f, 2.6f));
    g.setColour (juce::Colour (0xff959ca6));
    g.drawText ("SOUND WITH INTENTION", juce::Rectangle<float> (1178.0f, 27.0f, 182.0f, 14.0f),
                juce::Justification::centredRight, false);

    // ------------------------------------------------------------- signature
    g.setFont (juce::Font (juce::Font::getDefaultSerifFontName(), 62.0f, juce::Font::italic)
                 .withExtraKerningFactor (-0.02f));
    g.setColour (juce::Colour (0xfff3ece6));
    g.drawText ("Milad", juce::Rectangle<float> (40.0f, 686.0f, 360.0f, 84.0f),
                juce::Justification::centred, false);

    {
        juce::Graphics::ScopedSaveState state (g);
        g.addTransform (juce::AffineTransform::scale (1.55f, 1.0f)
                            .translated (62.0f, 764.0f));
        g.setColour (accent.withAlpha (0.75f));
        g.strokePath (signature, juce::PathStrokeType (1.1f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
    }

    g.setFont (labelFont (7.8f, 2.7f));
    g.setColour (juce::Colour (0xffcfc8c1));
    g.drawText ("MUSIC / PEOPLE / EMOTION", juce::Rectangle<float> (24.0f, 806.0f, 392.0f, 14.0f),
                juce::Justification::centred, false);
    g.setColour (juce::Colour (0xffb7b0aa));
    g.drawText ("ALWAYS", juce::Rectangle<float> (24.0f, 822.0f, 392.0f, 14.0f),
                juce::Justification::centred, false);

    g.setFont (labelFont (6.6f, 2.2f));
    g.setColour (juce::Colour (0xff7d858f));
    g.drawText ("VERSION 1.0.0", juce::Rectangle<float> (22.0f, 902.0f, 200.0f, 12.0f),
                juce::Justification::centredLeft, false);

    // ---------------------------------------------------------------- footer
    g.setColour (juce::Colour (0xff191c21));
    g.drawLine (0.0f, 938.0f, (float) designW, 938.0f, 1.0f);

    g.setFont (labelFont (6.8f, 2.2f));
    g.setColour (juce::Colour (0xff7d858f));
    g.drawText ("dB", juce::Rectangle<float> (16.0f, 942.0f, 40.0f, 14.0f),
                juce::Justification::centredLeft, false);
    g.drawText ("dB", juce::Rectangle<float> (1320.0f, 942.0f, 40.0f, 14.0f),
                juce::Justification::centredRight, false);

    g.setFont (labelFont (8.8f, 2.8f));
    g.setColour (juce::Colour (0xffcfd5dc));
    g.drawText ("AN ALL-IN-ONE MIXING ENVIRONMENT",
                juce::Rectangle<float> (448.0f, 942.0f, 852.0f, 14.0f),
                juce::Justification::centred, false);

    g.setFont (labelFont (6.8f, 2.2f));
    g.setColour (juce::Colour (0xff7d858f));
    g.drawText ("DESIGNED BY MILAD HASHEMI",
                juce::Rectangle<float> (1050.0f, 942.0f, 240.0f, 14.0f),
                juce::Justification::centredRight, false);
}

} // namespace mhui

//==============================================================================
MHSignatureAudioProcessorEditor::MHSignatureAudioProcessorEditor (MHSignatureAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), canvas (p)
{
    setOpaque (true);
    addAndMakeVisible (canvas);

    addAndMakeVisible (tooltips);
    tooltips.setAlwaysOnTop (true);

    setResizable (true, true);
    setResizeLimits (688, 480, 2752, 1920);

    if (auto* constrainer = getConstrainer())
        constrainer->setFixedAspectRatio ((double) mhui::Canvas::designW
                                          / (double) mhui::Canvas::designH);

    setSize (1100, 767);
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
    const float x = ((float) getWidth() - w) * 0.5f;
    const float y = ((float) getHeight() - h) * 0.5f;

    canvas.setTransform (juce::AffineTransform::scale (scale).translated (x, y));
    canvas.setBounds (0, 0, mhui::Canvas::designW, mhui::Canvas::designH);
}
