#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

/*==============================================================================
    MH SIGNATURE — the interface.

    Everything is laid out in a fixed 1440 x 900 design space and scaled as one
    unit, so the type is never squeezed.  The window can be dragged from 85 % to
    200 %; the design keeps its aspect ratio.

    The grid below is the same grid the design board was drawn on:

        portrait | input   | SIGNATURE ENGINE        | spectrum        | output
        ---------+---------+-------------------------+-----------------+-------
                 | EQ      | DYNAMICS                | SATURATION      |
                 | VOCAL   | REVERB | DELAY          | IMAGER          |
                 | MOD     | TRANSIENT               | I/O             |
==============================================================================*/
namespace mhui
{

//==============================================================================
//  Palette — design/tokens.json
//==============================================================================
inline const juce::Colour noir       { 0xff050607 };
inline const juce::Colour cardTop    { 0xff101216 };
inline const juce::Colour cardBottom { 0xff07080a };
inline const juce::Colour edgeCol    { 0xff292d34 };
inline const juce::Colour accent     { 0xffd0361c };
inline const juce::Colour deepAccent { 0xff8f2412 };
inline const juce::Colour softAccent { 0xfff2745a };
inline const juce::Colour ink        { 0xffeef1f5 };
inline const juce::Colour ink2       { 0xffcfd5dc };
inline const juce::Colour muted      { 0xff8e96a1 };
inline const juce::Colour dim        { 0xff626974 };
inline const juce::Colour faint      { 0xff444952 };
inline const juce::Colour trackCol   { 0xff2c3038 };
inline const juce::Colour gridCol    { 0xff1a1d23 };

enum class Fmt { int0, db1, pct0, hz, ms, sec1, ratio, raw1, dbInt };

juce::String formatValue (float v, Fmt f);

//==============================================================================
//  Type scale — big enough to read at 100 %
//==============================================================================
inline constexpr float typeLabel   = 10.0f;   // knob names
inline constexpr float typeValue   = 9.4f;    // knob values
inline constexpr float typeModule  = 10.4f;   // module titles
inline constexpr float typeSmall   = 8.2f;    // chips, tabs, read-outs

juce::Font uiFont (float size, bool bold = false, float kerning = 1.2f);
juce::Font displayFont (float size, bool bold = false, float kerning = 2.4f);

//==============================================================================
/** Shared painting: the black glass card every module sits on. */
namespace art
{
    void card (juce::Graphics&, juce::Rectangle<float> area, float radius = 8.0f,
               float glow = 0.0f);
    void glowArc (juce::Graphics&, float cx, float cy, float r,
                  float a0, float a1, juce::Colour, float width);
}

//==============================================================================
class MhKnob : public juce::Slider
{
public:
    MhKnob (MHSignatureAudioProcessor& proc,
            const juce::String& paramId,
            const juce::String& labelText,
            Fmt format,
            bool bigStyle = false,
            bool hideText = false);

    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit  (const juce::MouseEvent&) override;

    void setAccentGlow (float g);
    void setDimmed (bool shouldDim);

    juce::String knobLabel;

private:
    Fmt fmt;
    bool big = false;
    bool noText = false;
    bool dimmed = false;
    float glow = 0.0f;
    bool hovered = false;
    juce::RangedAudioParameter* param = nullptr;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MhKnob)
};

//==============================================================================
/** The per-module ON / OFF switch: a lamp in a pill that reads ON or OFF. */
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
/** A black glass module card: lamp + title + read-out in the header, optional
    artwork, then one row of named knobs. */
class ModulePanel : public juce::Component
{
public:
    ModulePanel (MHSignatureAudioProcessor& proc, const juce::String& title,
                 bool showSwitch = true);

    void addPower (const juce::String& paramId, const juce::String& shortName);
    void addKnob (const juce::String& id, const juce::String& label, Fmt fmt,
                  bool accentPointer = false);
    void setArt (std::function<void (juce::Graphics&, juce::Rectangle<float>)> drawer);
    void setReadout (juce::String text);
    void setCaption (juce::String text);
    void setGlow (float g);

    bool isAnyModuleOn() const;
    void refreshDimming();
    void setFocused (bool shouldBeFocused);

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::Rectangle<int> getBodyArea() const;
    juce::Rectangle<int> getArtArea() const;
    juce::Rectangle<int> headerArea() const;

    juce::String title;

    /** Extra placement for children that are not knobs (engine macros, chips). */
    std::function<void()> customLayout;

private:
    MHSignatureAudioProcessor& proc;
    std::vector<std::unique_ptr<MhKnob>> knobs;
    std::vector<std::unique_ptr<PowerSwitch>> powers;

    std::function<void (juce::Graphics&, juce::Rectangle<float>)> artDrawer;
    juce::String readout, caption;
    bool hasSwitch = true;
    bool focused = true;
    float glowAmount = 0.0f;

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
/** A tall thin level meter: its own card, a segmented bar, dB ticks and a
    numeric read-out. */
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
class SpectrumPanel : public juce::Component,
                      private juce::Timer
{
public:
    explicit SpectrumPanel (MHSignatureAudioProcessor& proc);
    ~SpectrumPanel() override;

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    void timerCallback() override;

    juce::Rectangle<float> plotArea() const;
    juce::Rectangle<float> tabRect (int i) const;
    juce::Rectangle<float> chipRect (int i) const;

    MHSignatureAudioProcessor& proc;
    int mode = 0;
    bool pre = false;

    juce::StringArray tabNames { "SPECTRUM", "WAVEFORM", "LOUDNESS" };

    std::array<float, MHSignatureAudioProcessor::numSpectrumBands> spec {}, specPre {};
    std::array<float, MHSignatureAudioProcessor::numScopeSamples> wave {};
    std::array<float, 128> hist {};
    int histIndex = 0, loudCounter = 0;
    float loudAccum = -90.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumPanel)
};

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
                                 vocPanel, spacePanel, delayPanel, imgPanel,
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
