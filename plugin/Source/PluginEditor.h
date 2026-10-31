#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

/*==============================================================================
    MH SIGNATURE — the interface.

    A page-based hardware desk.  Nothing is crammed onto one board: the ALL tab
    is the overview, and every other tab gives one module the whole desk.

        header   monogram · MH SIGNATURE · tabs · preset window
        rail     portrait (left) · signal chain + utility (right)
        desk     brushed plates, metal dials, a small LED window per control

    Everything is drawn in a fixed 1440 x 900 design space and scaled as one
    unit, so the type is never squeezed.  The window can be dragged from 85 %
    to 200 % and keeps its aspect ratio.
==============================================================================*/
namespace mhui
{

//==============================================================================
//  Palette — design/tokens.json
//==============================================================================
inline const juce::Colour noir       { 0xff070708 };
inline const juce::Colour plateTop   { 0xff191b1f };
inline const juce::Colour plateBot   { 0xff0d0e11 };
inline const juce::Colour plateEdge  { 0xff2e3239 };
inline const juce::Colour bevelHi    { 0xff42474f };
inline const juce::Colour ink        { 0xffeceff2 };
inline const juce::Colour ink2       { 0xffc6ccd3 };
inline const juce::Colour muted      { 0xff8c939d };
inline const juce::Colour dimText    { 0xff5f6670 };
inline const juce::Colour faint      { 0xff40454e };
inline const juce::Colour accent     { 0xffd0361c };
inline const juce::Colour deepAccent { 0xff882111 };
inline const juce::Colour softAccent { 0xfff2745a };
inline const juce::Colour ledBg      { 0xff0d0b0b };
inline const juce::Colour metalTop   { 0xff686c72 };
inline const juce::Colour metalBot   { 0xff282b30 };

enum class Fmt { int0, db1, pct0, hz, ms, sec1, ratio, raw1, dbInt };

juce::String formatValue (float v, Fmt f);

inline constexpr float typeLabel  = 7.8f;
inline constexpr float typeValue  = 7.2f;
inline constexpr float typeModule = 9.4f;
inline constexpr float typeSmall  = 6.4f;
inline constexpr float typeMicro  = 5.8f;

juce::Font uiFont (float size, bool bold = false, float kerning = 1.2f);
juce::Font monoFont (float size, float kerning = 0.2f);

//==============================================================================
/** Shared painting. */
namespace art
{
    /** A brushed-metal plate with a bevel and four screws. */
    void plate (juce::Graphics&, juce::Rectangle<float> area, float radius = 6.0f);
    void screws (juce::Graphics&, juce::Rectangle<float> area);
    /** The dark digital window a value is printed in. */
    void ledBox (juce::Graphics&, juce::Rectangle<float> area, const juce::String& text,
                 float size, juce::Colour textColour = softAccent);
    /** A rocker switch, drawn for PowerSwitch and for the static rail. */
    void rocker (juce::Graphics&, juce::Rectangle<float> area, bool on, bool hover);
    void text (juce::Graphics&, const juce::String&, juce::Rectangle<float> area,
               float size, juce::Colour, juce::Justification, bool bold = false,
               float kerning = 1.2f);
}

//==============================================================================
/** A metal dial: printed tick scale, a vermilion value arc, a brushed cap with
    a pointer, the parameter name underneath and its value in a small LED
    window.  Dragging up increases the value; double-click restores the default.
    The pointer angle is the same as the value arc, so nothing can read backwards. */
class MhKnob : public juce::Slider
{
public:
    MhKnob (MHSignatureAudioProcessor& proc,
            const juce::String& paramId,
            const juce::String& labelText,
            Fmt format,
            bool ticks = true,
            bool hideText = false);

    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit  (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    void setDimmed (bool shouldDim);

    /** called on every value change, so the plate can redraw its artwork */
    std::function<void()> onEdit;

    juce::String knobLabel;

private:
    Fmt fmt;
    bool showTicks = true;
    bool noText = false;
    bool dimmed = false;
    bool hovered = false;
    juce::RangedAudioParameter* param = nullptr;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MhKnob)
};

//==============================================================================
/** The per-module OFF switch.  A physical rocker: click flips the parameter,
    the lamp and the word next to it follow the parameter. */
class PowerSwitch : public juce::Button
{
public:
    PowerSwitch (MHSignatureAudioProcessor& proc,
                 const juce::String& paramId,
                 const juce::String& shortName,
                 bool compact = false);

    void paintButton (juce::Graphics&, bool over, bool down) override;

    /** Re-reads the parameter; called by the editor timer. */
    void refresh();

    std::function<void()> onToggle;

    juce::String moduleName;

private:
    juce::RangedAudioParameter* param = nullptr;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    bool compactStyle = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PowerSwitch)
};

//==============================================================================
/** One hardware module: plate, header with its rocker, optional artwork,
    optional chip, then its dials. */
class ModulePanel : public juce::Component
{
public:
    ModulePanel (MHSignatureAudioProcessor& proc, const juce::String& title);

    void addPower (const juce::String& paramId, const juce::String& shortName);
    void addKnob (const juce::String& id, const juce::String& label, Fmt fmt,
                  bool primary = true);
    void setArt (std::function<void (juce::Graphics&, juce::Rectangle<float>)> drawer);
    void setArtHeight (float h);
    void setReadout (juce::String text);
    void setChip (juce::String name, juce::String value);
    void setChipRow (bool reserve);
    void setRows (int rows);
    void setMaxRadius (float r);
    void setSections (juce::StringArray labels);

    /** true = the ALL page: only the primary dials of the module are shown */
    void setOverview (bool on);

    void paint (juce::Graphics&) override;
    void resized() override;

    bool isAnyModuleOn() const;
    void refreshPowers();

    juce::String title;
    std::function<void()> customLayout;

private:
    juce::Rectangle<int> dialBox() const;
    juce::Rectangle<int> artBand() const;

    MHSignatureAudioProcessor& proc;
    std::vector<std::unique_ptr<MhKnob>> knobs;
    std::vector<std::unique_ptr<PowerSwitch>> powers;

    std::function<void (juce::Graphics&, juce::Rectangle<float>)> artDrawer;
    juce::String readout, chipName, chipValue;
    juce::StringArray sections;
    std::vector<bool> knobPrimary;
    bool overview = false;
    int rowsWanted = 1;
    float maxR = 22.0f;
    float artHeight = 0.0f;
    bool chipRow = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulePanel)
};

//==============================================================================
class ChoiceChip : public juce::Component, public juce::SettableTooltipClient
{
public:
    ChoiceChip (MHSignatureAudioProcessor& proc, const juce::String& paramId,
                const juce::String& caption);

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;

    /** follows the parameter; called by the editor timer */
    void refresh();

private:
    MHSignatureAudioProcessor& proc;
    juce::RangedAudioParameter* param = nullptr;
    juce::String caption;
    juce::String shown;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoiceChip)
};

//==============================================================================
/** Spectrum / waveform / loudness screen with a live statistics row. */
class AnalyserPanel : public juce::Component,
                      private juce::Timer
{
public:
    explicit AnalyserPanel (MHSignatureAudioProcessor& proc);
    ~AnalyserPanel() override;

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;

    void setStatsVisible (bool shouldShow);

private:
    void timerCallback() override;

    juce::Rectangle<float> plotArea() const;
    juce::Rectangle<float> tabRect (int i) const;
    juce::Rectangle<float> chipRect (int i) const;

    MHSignatureAudioProcessor& proc;
    int mode = 0;
    bool pre = false;
    bool stats = true;

    juce::StringArray tabNames { "SPECTRUM", "WAVE", "LOUDNESS" };

    std::array<float, MHSignatureAudioProcessor::numSpectrumBands> spec {}, specPre {};
    std::array<float, MHSignatureAudioProcessor::numScopeSamples> wave {};
    std::array<float, 128> hist {};
    int histIndex = 0, loudCounter = 0;
    float loudAccum = -90.0f;
    float peakHold = -90.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserPanel)
};

//==============================================================================
/** The right-hand rail: the order of the chain, a lamp per module that shows
    whether it is actually in the chain, and the utility section. */
class ChainRail : public juce::Component
{
public:
    explicit ChainRail (MHSignatureAudioProcessor& proc);
    ~ChainRail() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void setActive (const juce::String& moduleName);
    void refresh();

private:
    MHSignatureAudioProcessor& proc;
    juce::String active;

    std::unique_ptr<MhKnob> inKnob, outKnob;
    std::unique_ptr<PowerSwitch> monoSwitch, bypassSwitch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChainRail)
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
    void setIndex (int i) { current = i; repaint(); }

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

    /** follows the preset book from the editor's timer */
    void refresh();

private:
    void showBrowser();

    MHSignatureAudioProcessor& proc;
    int hoverZone = 0;
    juce::String lastShown;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBar)
};

//==============================================================================
class Canvas : public juce::Component,
               private juce::Timer
{
public:
    explicit Canvas (MHSignatureAudioProcessor& proc);
    ~Canvas() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void timerCallback() override;

    static constexpr int designW = 1440;
    static constexpr int designH = 900;

private:
    enum class Page { all = 0, eq, dyn, colour, space, mod, image };

    void showPage (Page p);
    void buildLayout();

    MHSignatureAudioProcessor& proc;
    juce::Image portrait;
    Page page = Page::all;

    std::unique_ptr<ModulePanel> enginePanel, eqPanel, dynPanel, satPanel, vocPanel,
                                 revPanel, dlyPanel, modPanel, trnPanel, imgPanel,
                                 bandPanel;
    std::vector<std::unique_ptr<MhKnob>> engineMacros;
    std::unique_ptr<MhKnob> engineKnob;
    std::unique_ptr<ChoiceChip> satType;
    std::unique_ptr<AnalyserPanel> analyser;
    std::unique_ptr<ChainRail> rail;
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
    juce::TooltipWindow tooltips { nullptr, 900 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MHSignatureAudioProcessorEditor)
};
