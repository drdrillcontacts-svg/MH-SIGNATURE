#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

/*==============================================================================
    The MH SIGNATURE interface.

    Everything is laid out in a fixed 1440 × 900 design space at readable sizes
    (11 px labels, 12 px module titles) and scaled as one unit.  The window can
    be resized from 85 % to 200 %; the design is never squeezed below 85 %, so
    type stays legible.
==============================================================================*/
namespace mhui
{

//==============================================================================
//  Palette — design/tokens.json
//==============================================================================
inline const juce::Colour noir       { 0xff050607 };
inline const juce::Colour panelBg    { 0xff101216 };
inline const juce::Colour edgeCol    { 0xff24272d };
inline const juce::Colour accent     { 0xffd0361c };
inline const juce::Colour deepAccent { 0xff8f2412 };
inline const juce::Colour softAccent { 0xfff2745a };
inline const juce::Colour ink        { 0xffeef1f5 };
inline const juce::Colour muted      { 0xff8a929e };
inline const juce::Colour gridCol    { 0xff1d2026 };

enum class Fmt { int0, db1, pct0, hz, ms, sec1, ratio, raw1, dbInt };

juce::String formatValue (float v, Fmt f);

//==============================================================================
//  Type scale — big enough to read at 100 %
//==============================================================================
inline constexpr float typeLabel  = 10.5f;   // knob names
inline constexpr float typeValue  = 9.5f;    // knob values
inline constexpr float typeModule = 11.5f;   // module titles
inline constexpr float typeSmall  = 9.0f;    // chips, tabs, hints

juce::Font uiFont     (float size, bool bold = false, float kerning = 1.2f);
juce::Font displayFont(float size, bool bold = false, float kerning = 2.4f);

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

    void setAccentGlow (float g);
    void setDimmed (bool shouldDim);

    juce::String knobLabel;

private:
    Fmt fmt;
    bool big;
    bool dimmed = false;
    float glow = 0.0f;
    bool hovered = false;
    juce::RangedAudioParameter* param = nullptr;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MhKnob)
};

//==============================================================================
/** The per-module OFF switch.  A pill that reads ON or OFF, so the state of
    every module is obvious without hovering. */
class PowerSwitch : public juce::Button
{
public:
    PowerSwitch (MHSignatureAudioProcessor& proc,
                 const juce::String& paramId,
                 const juce::String& shortName);

    void paintButton (juce::Graphics&, bool over, bool down) override;

    std::function<void()> onToggle;

    juce::String moduleName;

private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PowerSwitch)
};

//==============================================================================
/** A bordered module card: title, ON/OFF switches, optional artwork, a row of
    named knobs. */
class ModulePanel : public juce::Component
{
public:
    ModulePanel (MHSignatureAudioProcessor& proc, const juce::String& title);

    void addPower (const juce::String& paramId, const juce::String& shortName);
    void addKnob (const juce::String& id, const juce::String& label, Fmt fmt);
    void setArt (int artHeight,
                 std::function<void (juce::Graphics&, juce::Rectangle<float>)> drawer);
    void setCaption (const juce::String& text, bool drawOnTop = false);

    bool isAnyModuleOn() const;
    void refreshDimming();

    void setFocused (bool shouldBeFocused);

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::Rectangle<int> getBodyArea() const;
    juce::Rectangle<int> getArtArea() const;
    juce::Rectangle<int> getKnobArea() const;

    juce::String title;

    /** Extra placement for children that are not knobs (engine macros, chips). */
    std::function<void()> customLayout;

private:
    MHSignatureAudioProcessor& proc;
    std::vector<std::unique_ptr<MhKnob>> knobs;
    std::vector<std::unique_ptr<PowerSwitch>> powers;

    std::function<void (juce::Graphics&, juce::Rectangle<float>)> artDrawer;
    int artH = 0;
    juce::String caption;
    bool captionOnTop = false;
    bool focused = true;

    juce::Rectangle<int> headerArea() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulePanel)
};

//==============================================================================
/** Cycles a choice parameter on click. */
class ChoiceChip : public juce::Component, public juce::SettableTooltipClient
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
class TabStrip : public juce::Component
{
public:
    TabStrip();
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    std::function<void (int)> onTabChanged;
    int getIndex() const { return current; }

private:
    juce::StringArray items { "ALL", "EQ", "DYN", "COLOR", "SPACE", "MOD", "IMAGE" };
    int current = 0;
    int hovered = -1;

    juce::Rectangle<float> tabBounds (int i) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabStrip)
};

//==============================================================================
class PresetBar : public juce::Component, public juce::SettableTooltipClient
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
    int hoverZone = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBar)
};

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
    void timerCallback() override;

    static constexpr int designW = 1440;
    static constexpr int designH = 900;

private:
    void buildLayout();
    void applyFocus (int tab);

    MHSignatureAudioProcessor& proc;
    juce::Image portrait;
    float lfoPhase = 0.0f;
    float engineGlow = 0.0f;

    std::unique_ptr<ModulePanel> enginePanel, eqPanel, dynPanel, satPanel,
                                 vocPanel, spacePanel, imgPanel,
                                 modPanel, trnPanel, ioPanel;
    std::vector<std::unique_ptr<MhKnob>> engineMacros;
    std::unique_ptr<MhKnob> engineKnob;
    std::unique_ptr<ChoiceChip> satType;
    std::unique_ptr<PowerSwitch> monoSwitch, bypassSwitch;
    std::unique_ptr<LevelMeter> inMeter, outMeter;
    std::unique_ptr<TabStrip> tabs;
    std::unique_ptr<PresetBar> presetBar;
    std::unique_ptr<SpectrumPanel> spectrumPanel;

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
    juce::TooltipWindow tooltips { nullptr, 900 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MHSignatureAudioProcessorEditor)
};
