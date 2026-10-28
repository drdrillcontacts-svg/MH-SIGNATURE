#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

/*==============================================================================
    Parameter identifiers.

    Every default below is lifted from the analysis of 22 mixes by Milad
    Hashemi (see analysis/spec/spec-10-modules.html).  The four macros
    (character / density / space / width) are the intended primary controls;
    every module knob underneath is a refinement of those four.
==============================================================================*/

namespace Params
{
    // ---------------------------------------------------------- signature engine
    inline constexpr const char* character  = "sig_character";
    inline constexpr const char* density    = "sig_density";
    inline constexpr const char* space      = "sig_space";
    inline constexpr const char* width      = "sig_width";
    inline constexpr const char* intensity  = "sig_intensity";

    // ------------------------------------------------------------------ equaliser
    inline constexpr const char* eqOn       = "eq_on";
    inline constexpr const char* eqHpf      = "eq_hpf";
    inline constexpr const char* eqLowF     = "eq_low_f";
    inline constexpr const char* eqLowG     = "eq_low_g";
    inline constexpr const char* eqLmF      = "eq_lm_f";
    inline constexpr const char* eqLmG      = "eq_lm_g";
    inline constexpr const char* eqHmF      = "eq_hm_f";
    inline constexpr const char* eqHmG      = "eq_hm_g";
    inline constexpr const char* eqHighF    = "eq_high_f";
    inline constexpr const char* eqHighG    = "eq_high_g";
    inline constexpr const char* eqTilt     = "eq_tilt";

    // ------------------------------------------------------------------ dynamics
    inline constexpr const char* dynOn      = "dyn_on";
    inline constexpr const char* dynThr     = "dyn_thr";
    inline constexpr const char* dynRatio   = "dyn_ratio";
    inline constexpr const char* dynAtt     = "dyn_att";
    inline constexpr const char* dynRel     = "dyn_rel";
    inline constexpr const char* dynMakeup  = "dyn_makeup";
    inline constexpr const char* dynMix     = "dyn_mix";
    inline constexpr const char* limOn      = "lim_on";
    inline constexpr const char* limCeiling = "lim_ceiling";
    inline constexpr const char* limClip    = "lim_clip";

    // --------------------------------------------------------------------- vocal
    inline constexpr const char* vocOn      = "voc_on";
    inline constexpr const char* vocHpf     = "voc_hpf";
    inline constexpr const char* vocBodyG   = "voc_body_g";
    inline constexpr const char* vocThr     = "voc_comp_thr";
    inline constexpr const char* vocRatio   = "voc_comp_ratio";
    inline constexpr const char* vocDeessF  = "voc_deess_f";
    inline constexpr const char* vocDeessR  = "voc_deess_range";
    inline constexpr const char* vocPresG   = "voc_presence_g";
    inline constexpr const char* vocAirG    = "voc_air_g";
    inline constexpr const char* vocSendRev = "voc_send_rev";

    // ---------------------------------------------------------- saturation/colour
    inline constexpr const char* satOn      = "sat_on";
    inline constexpr const char* satType    = "sat_type";
    inline constexpr const char* satDrive   = "sat_drive";
    inline constexpr const char* satTone    = "sat_tone";
    inline constexpr const char* satHf      = "sat_hf";
    inline constexpr const char* satMix     = "sat_mix";

    // -------------------------------------------------------------- delay/reverb
    inline constexpr const char* revOn      = "rev_on";
    inline constexpr const char* revSize    = "rev_size";
    inline constexpr const char* revDecay   = "rev_decay";
    inline constexpr const char* revDamp    = "rev_damp";
    inline constexpr const char* revPre     = "rev_predelay";
    inline constexpr const char* revMix     = "rev_mix";
    inline constexpr const char* dlyOn      = "dly_on";
    inline constexpr const char* dlyTime    = "dly_time";
    inline constexpr const char* dlyFb      = "dly_fb";
    inline constexpr const char* dlyFilter  = "dly_filter";
    inline constexpr const char* dlyMix     = "dly_mix";
    inline constexpr const char* dlyPingPong= "dly_pp";

    // -------------------------------------------------------------------- imager
    inline constexpr const char* imgOn      = "img_on";
    inline constexpr const char* imgMonoF   = "img_mono_f";
    inline constexpr const char* imgWidth   = "img_width";
    inline constexpr const char* imgWLow    = "img_w_low";
    inline constexpr const char* imgWMid    = "img_w_mid";
    inline constexpr const char* imgWHigh   = "img_w_high";
    inline constexpr const char* imgWAir    = "img_w_air";
    inline constexpr const char* imgBalance = "img_balance";

    // ---------------------------------------------------------------- modulation
    inline constexpr const char* modOn      = "mod_on";
    inline constexpr const char* modRate    = "mod_rate";
    inline constexpr const char* modDepth   = "mod_depth";
    inline constexpr const char* modMix     = "mod_mix";

    // ---------------------------------------------------------- transient shaper
    inline constexpr const char* trnOn      = "trn_on";
    inline constexpr const char* trnAtt     = "trn_att";
    inline constexpr const char* trnSus     = "trn_sus";
    inline constexpr const char* trnFreq    = "trn_freq";

    // ----------------------------------------------------------------- utility
    inline constexpr const char* ioInput    = "io_input";
    inline constexpr const char* ioOutput   = "io_output";
    inline constexpr const char* ioMix      = "io_mix";
    inline constexpr const char* ioMono     = "io_mono";
    inline constexpr const char* ioBypass   = "io_bypass";

    //==========================================================================
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        AudioProcessorValueTreeState::ParameterLayout layout;

        auto pid = [] (const char* id) { return ParameterID { id, 1 }; };

        auto addFloat = [&] (const char* id, const String& name,
                             Range<float> range, float def, const String& unit,
                             float skew = 1.0f)
        {
            layout.add (std::make_unique<AudioParameterFloat> (
                pid (id), name, NormalisableRange<float> (range.start, range.end, 0.0f, skew),
                def, AudioParameterFloatAttributes().withLabel (unit)));
        };

        auto addBool = [&] (const char* id, const String& name, bool def)
        {
            layout.add (std::make_unique<AudioParameterBool> (pid (id), name, def));
        };

        auto addChoice = [&] (const char* id, const String& name,
                              const StringArray& items, int def)
        {
            layout.add (std::make_unique<AudioParameterChoice> (pid (id), name, items, def));
        };

        // ---------------------------------------------------- signature engine
        addFloat (character, "Character",    { 0.0f, 100.0f },  68.0f, "%");
        addFloat (density,   "Density",      { 0.0f, 100.0f },  62.0f, "%");
        addFloat (space,     "Space",        { 0.0f, 100.0f },  70.0f, "%");
        addFloat (width,     "Width",        { 0.0f, 100.0f },  38.0f, "%");
        addFloat (intensity, "Intensity",    { 0.0f, 100.0f }, 100.0f, "%");

        // ------------------------------------------------------------- equaliser
        addBool  (eqOn,   "EQ On", true);
        addFloat (eqHpf,  "HPF",             { 20.0f, 200.0f },   32.0f, "Hz", 0.35f);
        addFloat (eqLowF, "Low Freq",        { 40.0f, 200.0f },   65.0f, "Hz", 0.4f);
        addFloat (eqLowG, "Low Gain",        { -18.0f, 18.0f },    3.0f, "dB");
        addFloat (eqLmF,  "Low Mid Freq",    { 100.0f, 600.0f }, 152.0f, "Hz", 0.4f);
        addFloat (eqLmG,  "Low Mid Gain",    { -18.0f, 18.0f },   -2.4f, "dB");
        addFloat (eqHmF,  "High Mid Freq",   { 1800.0f, 8000.0f }, 3500.0f, "Hz", 0.4f);
        addFloat (eqHmG,  "High Mid Gain",   { -18.0f, 18.0f },   -3.5f, "dB");
        addFloat (eqHighF,"High Freq",       { 5000.0f, 16000.0f }, 9000.0f, "Hz", 0.4f);
        addFloat (eqHighG,"High Gain",       { -24.0f, 12.0f },   -4.0f, "dB");
        addFloat (eqTilt, "Tilt",            { -6.0f, 3.0f },     -1.0f, "dB/oct");

        // -------------------------------------------------------------- dynamics
        addBool  (dynOn,     "Comp On", true);
        addFloat (dynThr,    "Threshold",    { -60.0f, 0.0f },  -18.0f, "dB");
        addFloat (dynRatio,  "Ratio",        { 1.0f, 20.0f },     4.0f, ":1", 0.5f);
        addFloat (dynAtt,    "Attack",       { 0.1f, 100.0f },   10.0f, "ms", 0.35f);
        addFloat (dynRel,    "Release",      { 10.0f, 1000.0f }, 100.0f, "ms", 0.35f);
        addFloat (dynMakeup, "Makeup",       { 0.0f, 24.0f },      3.2f, "dB");
        addFloat (dynMix,    "Mix",          { 0.0f, 100.0f },   100.0f, "%");
        addBool  (limOn,     "Limiter On", true);
        addFloat (limCeiling,"Ceiling",      { -6.0f, 0.0f },     -0.3f, "dBFS");
        addFloat (limClip,   "Clip",         { 0.0f, 6.0f },       1.5f, "dB");

        // ----------------------------------------------------------------- vocal
        addBool  (vocOn,     "Vocal On", true);
        addFloat (vocHpf,    "Vocal HPF",    { 40.0f, 250.0f },   95.0f, "Hz", 0.4f);
        addFloat (vocBodyG,  "Body",         { -12.0f, 12.0f },    2.0f, "dB");
        addFloat (vocThr,    "Vocal Threshold", { -60.0f, 0.0f }, -22.0f, "dB");
        addFloat (vocRatio,  "Vocal Ratio",  { 1.0f, 20.0f },      4.0f, ":1", 0.5f);
        addFloat (vocDeessF, "De-Ess Freq",  { 4000.0f, 10000.0f }, 6500.0f, "Hz", 0.4f);
        addFloat (vocDeessR, "De-Ess Range", { -18.0f, 0.0f },    -4.5f, "dB");
        addFloat (vocPresG,  "Presence",     { -12.0f, 12.0f },   -1.5f, "dB");
        addFloat (vocAirG,   "Air",          { -18.0f, 6.0f },    -6.0f, "dB");
        addFloat (vocSendRev,"Vocal → Reverb", { 0.0f, 100.0f },  25.0f, "%");

        // ----------------------------------------------------- saturation / colour
        addBool  (satOn,   "Colour On", true);
        addChoice(satType, "Character", { "Tape", "Console", "Tube", "Diode", "Hard Clip" }, 1);
        addFloat (satDrive,"Drive",          { 0.0f, 24.0f },      4.5f, "dB");
        addFloat (satTone, "Tone",           { 0.0f, 100.0f },    37.0f, "%");
        addFloat (satHf,   "HF Rolloff",     { 6000.0f, 20000.0f }, 9500.0f, "Hz", 0.5f);
        addFloat (satMix,  "Mix",            { 0.0f, 100.0f },    58.0f, "%");

        // ---------------------------------------------------------- delay / reverb
        addBool  (revOn,   "Reverb On", true);
        addFloat (revSize, "Size",           { 0.0f, 100.0f },      62.0f, "%");
        addFloat (revDecay,"Decay",          { 0.3f, 12.0f },        3.4f, "s", 0.5f);
        addFloat (revDamp, "Damping",        { 0.0f, 100.0f },      70.0f, "%");
        addFloat (revPre,  "Pre-Delay",      { 0.0f, 200.0f },      26.0f, "ms");
        addFloat (revMix,  "Reverb Mix",     { 0.0f, 100.0f },      24.0f, "%");
        addBool  (dlyOn,   "Delay On", true);
        addFloat (dlyTime, "Time",           { 10.0f, 2000.0f },   375.0f, "ms", 0.5f);
        addFloat (dlyFb,   "Feedback",       { 0.0f, 95.0f },       36.0f, "%");
        addFloat (dlyFilter,"Filter",        { 1000.0f, 20000.0f }, 7200.0f, "Hz", 0.5f);
        addFloat (dlyMix,  "Delay Mix",      { 0.0f, 100.0f },      18.0f, "%");
        addBool  (dlyPingPong, "Ping-Pong", true);

        // ------------------------------------------------------------------ imager
        addBool  (imgOn,     "Imager On", true);
        addFloat (imgMonoF,  "Mono Below",   { 20.0f, 300.0f },     92.0f, "Hz", 0.4f);
        addFloat (imgWidth,  "Width",        { 0.0f, 200.0f },     132.0f, "%");
        addFloat (imgWLow,   "Width Low",    { 0.0f, 200.0f },       0.0f, "%");
        addFloat (imgWMid,   "Width Mid",    { 0.0f, 200.0f },      55.0f, "%");
        addFloat (imgWHigh,  "Width High",   { 0.0f, 200.0f },      85.0f, "%");
        addFloat (imgWAir,   "Width Air",    { 0.0f, 200.0f },     120.0f, "%");
        addFloat (imgBalance,"Balance",      { -100.0f, 100.0f },    0.0f, "%");

        // -------------------------------------------------------------- modulation
        addBool  (modOn,   "Modulation On", true);
        addFloat (modRate, "Rate",           { 0.05f, 10.0f },       0.30f, "Hz", 0.4f);
        addFloat (modDepth,"Depth",          { 0.0f, 100.0f },       28.0f, "%");
        addFloat (modMix,  "Mix",            { 0.0f, 100.0f },       16.0f, "%");

        // ---------------------------------------------------------- transient shaper
        addBool  (trnOn,   "Transient On", true);
        addFloat (trnAtt,  "Attack",         { -100.0f, 100.0f },    12.0f, "%");
        addFloat (trnSus,  "Sustain",        { -100.0f, 100.0f },    -8.0f, "%");
        addFloat (trnFreq, "Split Freq",     { 40.0f, 2000.0f },    180.0f, "Hz", 0.4f);

        // ----------------------------------------------------------------- utility
        addFloat (ioInput,  "Input",         { -24.0f, 24.0f },      -3.2f, "dB");
        addFloat (ioOutput, "Output",        { -24.0f, 24.0f },      -1.0f, "dB");
        addFloat (ioMix,    "Mix",           { 0.0f, 100.0f },      100.0f, "%");
        addBool  (ioMono,   "Mono", false);
        addBool  (ioBypass, "Bypass", false);

        return layout;
    }
}
