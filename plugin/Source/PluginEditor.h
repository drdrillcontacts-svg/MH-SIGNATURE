#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

/*==============================================================================
    MIRROR · D — the MH SIGNATURE interface.

    Everything is laid out in a fixed 1376 × 960 design space and scaled as one
    unit, so the window can be resized from 50 % to 200 % without the layout
    ever breaking.
==============================================================================*/
namespace mhui
{

//==============================================================================
//  Palette — taken from design/tokens.json
//==============================================================================
inline const juce::Colour noir       { 0xff050607 };
inline const juce::Colour panelBg    { 0xff101216 };
inline const juce::Colour panelBg2   { 0xff171a1f };
inline const juce::Colour edgeCol    { 0xff24272d };
inline const juce::Colour accent     { 0xffd0361c };
inline const juce::Colour deepAccent { 0xff8f2412 };
inline const juce::Colour softAccent { 0xfff2745a };
inline const juce::Colour ink        { 0xffeef1f5 };
inline const juce::Colour muted      { 0xff8a929e };
inline const juce::Colour gridCol    { 0xff1d2026 };

enum class Fmt { int0, db1, pct0, hz, ms, sec1, ratio, raw1, dbInt };

juce::String formatValue (float v, Fmt f);

juce::Font labelFont  (float size, float kerning = 1.6f);
juce::Font numberFont (float size, float kerning = 0.6f);
juce::Font titleFont  (float size, float kerning = 2.4f);

//==============================================================================
//  Small pieces
//==============================================================================
class MhKnob : public juce::Slider
{
public:
    MhKnob (MHSignatureAudioProcessor& proc,
            const juce::String& paramId,
            const juce::String& labelText,
            Fmt format,
            bool bigStyle = false);

    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit  (const juce::MouseEvent&) override;

    void setAccentGlow (float g)
    {
        const float ng = juce::jlimit (0.0f, 1.0f, g);
        if (std::abs (ng - glow) > 0.015f) { glow = ng; repaint(); }
    }

private:
    juce::String label;
    Fmt fmt;
    bool big;
    float glow = 0.0f;
    bool hovered = false;
    juce::RangedAudioParameter* param = nullptr;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MhKnob)
};

//==============================================================================
/** A tiny on/off dot that drives a bool parameter. */
class MhDot : public juce::Button
{
public:
    MhDot (MHSignatureAudioProcessor& proc, const juce::String& paramId, float diameter);

    void paintButton (juce::Graphics&, bool, bool) override;

private:
    juce::RangedAudioParameter* param = nullptr;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    float d;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MhDot)
};

//==============================================================================
/** A bordered panel with a title, an optional on/off dot, a reset gear and an
    optional piece of module artwork above its knob row. */
class ModulePanel : public juce::Component
{
public:
    ModulePanel (MHSignatureAudioProcessor& proc,
                 const juce::String& title,
                 const juce::StringArray& resetIds);

    juce::Rectangle<int> getBodyArea() const;
    juce::Rectangle<int> getArtArea (int artHeight) const;
    juce::Rectangle<int> getKnobArea (int artHeight) const;

    void setArt (int artHeight, std::function<void (juce::Graphics&, juce::Rectangle<float>)> drawer);

    void addKnob (const juce::String& id, const juce::String& label, Fmt fmt);
    MhKnob* getKnob (int i) { return juce::isPositiveAndBelow (i, (int) knobs.size()) ? knobs[(size_t) i].get() : nullptr; }

    void addDot (const juce::String& id);
    MhDot* getDot() { return dot.get(); }

    /** Dims panels that do not belong to the selected tab. */
    void setFocused (bool shouldBeFocused);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

    juce::String panelTitle;
    bool centreTitle = false;
    juce::String caption;

private:
    MHSignatureAudioProcessor& proc;
    juce::StringArray resetParams;
    std::vector<std::unique_ptr<MhKnob>> knobs;
    std::unique_ptr<MhDot> dot;
    int artH = 0;
    std::function<void (juce::Graphics&, juce::Rectangle<float>)> artDrawer;
    bool focused = true;
    bool gearHover = false;

    juce::Rectangle<int> gearBounds() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulePanel)
};

//==============================================================================
/** Cycles a choice parameter on click; used for the saturation character. */
class ChoiceChip : public juce::Component
{
public:
    ChoiceChip (MHSignatureAudioProcessor& proc, const juce::String& paramId,
                const juce::String& caption);

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    MHSignatureAudioProcessor& proc;
    juce::RangedAudioParameter* param = nullptr;
    juce::String caption;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoiceChip)
};

//==============================================================================
/** Vertical input / output meter with a dB scale. */
class LevelMeter : public juce::Component
{
public:
    LevelMeter (MHSignatureAudioProcessor& proc, bool inputSide);

    void paint (juce::Graphics&) override;

private:
    MHSignatureAudioProcessor& proc;
    bool isInput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};

//==============================================================================
/** Header tab strip with an accent pill on the active tab. */
class TabStrip : public juce::Component
{
public:
    TabStrip();
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;

    std::function<void (int)> onTabChanged;
    int getIndex() const { return current; }
    void setIndex (int i, bool notify = true);

private:
    juce::StringArray items { "ALL", "EQ", "DYN", "COLOR", "SPACE", "MOD", "IMAGE" };
    int current = 0;
    int hovered = -1;

    juce::Rectangle<float> tabBounds (int i) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabStrip)
};

//==============================================================================
/** The preset pill: chevrons for stepping, body opens the browser menu. */
class PresetBar : public juce::Component
{
public:
    explicit PresetBar (MHSignatureAudioProcessor& proc);

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    void showBrowser();

    MHSignatureAudioProcessor& proc;
    int hoverZone = 0;   // -1 prev, +1 next, 0 body

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBar)
};

//==============================================================================
/** The signature "Milad" mark, drawn as a path so it scales cleanly. */
juce::Path makeSignaturePath();

//==============================================================================
class SpectrumPanel;

//==============================================================================
class Canvas : public juce::Component,
               private juce::Timer
{
public:
    explicit Canvas (MHSignatureAudioProcessor& proc);
    ~Canvas() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    static constexpr int designW = 1376;
    static constexpr int designH = 960;

private:
    void buildLayout();
    void applyFocus (int tab);

    MHSignatureAudioProcessor& proc;
    juce::Image portrait;
    juce::Path signature;
    float lfoPhase = 0.0f;
    float engineGlow = 0.0f;

    std::unique_ptr<MhKnob> engineKnob;
    std::vector<std::unique_ptr<MhKnob>> engineMacros;
    std::unique_ptr<ModulePanel> enginePanel, eqPanel, compPanel, satPanel,
                                 revPanel, dlyPanel, modPanel, imgPanel,
                                 vocPanel, trnPanel, utilPanel;
    std::unique_ptr<SpectrumPanel> spectrumPanel;
    std::unique_ptr<ChoiceChip> satType;
    std::unique_ptr<LevelMeter> inMeter, outMeter;
    std::unique_ptr<MhDot> monoDot, bypassDot;
    std::unique_ptr<TabStrip> tabs;
    std::unique_ptr<PresetBar> presetBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Canvas)
};

} // namespace mhui

//==============================================================================
class MHSignatureAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MHSignatureAudioProcessorEditor (MHSignatureAudioProcessor&);
    ~MHSignatureAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MHSignatureAudioProcessor& proc;

    mhui::Canvas canvas;
    juce::TooltipWindow tooltips { nullptr, 800 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MHSignatureAudioProcessorEditor)
};
