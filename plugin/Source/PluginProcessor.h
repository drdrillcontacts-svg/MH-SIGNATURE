#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

#include "Parameters.h"
#include "DspChain.h"
#include "Presets.h"
#include "Analyzer.h"

/*==============================================================================
    MH SIGNATURE — audio processor.

    Ten modules around the Signature Engine.  The chain lives in DspChain.h;
    this class owns the parameter tree, the preset book, the meters and the
    analyser the editor draws.
==============================================================================*/
class MHSignatureAudioProcessor : public juce::AudioProcessor
{
public:
    MHSignatureAudioProcessor();
    ~MHSignatureAudioProcessor() override;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                       { return true; }

    const juce::String getName() const override           { return "MH SIGNATURE"; }
    bool acceptsMidi() const override                     { return false; }
    bool producesMidi() const override                    { return false; }
    bool isMidiEffect() const override                    { return false; }
    double getTailLengthSeconds() const override          { return 8.0; }

    //==========================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override                      { return currentPreset; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    //==========================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    juce::AudioProcessorValueTreeState apvts;

    /** Factory preset book (100 entries). */
    static int getNumFactoryPresets()                     { return (int) getFactoryPresets().size(); }
    static juce::String getFactoryPresetName (int index);
    static juce::String getFactoryPresetCategory (int index);

    void applyPreset (int index);
    void nextPreset()   { applyPreset ((currentPreset + 1) % juce::jmax (1, getNumFactoryPresets())); }
    void prevPreset()   { applyPreset ((currentPreset - 1 + getNumFactoryPresets()) % juce::jmax (1, getNumFactoryPresets())); }

    int  getCurrentPresetIndex() const                    { return currentPreset.load(); }
    juce::String getCurrentPresetName() const             { return presetName; }

    /** True when a knob has moved since the current preset was loaded. */
    bool isPresetEdited() const;

    //==========================================================================
    /** Live values for the editor (all lock free). */
    float getInputLevelDb()  const                        { return inputLevelDb.load(); }
    float getOutputLevelDb() const                        { return outputLevelDb.load(); }
    float getGainReductionDb() const                      { return grDb.load(); }
    float getEngineActivity() const                       { return engineActivity.load(); }

    /** Smoothed spectrum, magnitudes in dB, index 0 = 20 Hz … last = 20 kHz.
        @param pre  true = signal entering the plugin, false = signal leaving it. */
    static constexpr int numSpectrumBands = mh::Analyzer::numBands;
    void getSpectrum (float* dest, bool pre = false) const;

    /** 256 point oscilloscope ring, values in -1 … 1. */
    static constexpr int numScopeSamples = 256;
    void getWaveform (float* dest) const;

    /** Called by the editor when it rebuilds so meters can restart. */
    void resetMeters();

    //==========================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        return Params::createLayout();
    }

private:
    //==========================================================================
    struct Ptrs
    {
        std::atomic<float>* character = nullptr;
        std::atomic<float>* density = nullptr;
        std::atomic<float>* space = nullptr;
        std::atomic<float>* width = nullptr;
        std::atomic<float>* intensity = nullptr;

        std::atomic<float>* eqOn = nullptr;
        std::atomic<float>* eqHpf = nullptr;
        std::atomic<float>* eqLowF = nullptr;
        std::atomic<float>* eqLowG = nullptr;
        std::atomic<float>* eqLmF = nullptr;
        std::atomic<float>* eqLmG = nullptr;
        std::atomic<float>* eqHmF = nullptr;
        std::atomic<float>* eqHmG = nullptr;
        std::atomic<float>* eqHighF = nullptr;
        std::atomic<float>* eqHighG = nullptr;
        std::atomic<float>* eqTilt = nullptr;

        std::atomic<float>* dynOn = nullptr;
        std::atomic<float>* dynThr = nullptr;
        std::atomic<float>* dynRatio = nullptr;
        std::atomic<float>* dynAtt = nullptr;
        std::atomic<float>* dynRel = nullptr;
        std::atomic<float>* dynMakeup = nullptr;
        std::atomic<float>* dynMix = nullptr;
        std::atomic<float>* limOn = nullptr;
        std::atomic<float>* limCeiling = nullptr;
        std::atomic<float>* limClip = nullptr;

        std::atomic<float>* vocOn = nullptr;
        std::atomic<float>* vocHpf = nullptr;
        std::atomic<float>* vocBodyG = nullptr;
        std::atomic<float>* vocThr = nullptr;
        std::atomic<float>* vocRatio = nullptr;
        std::atomic<float>* vocDeessF = nullptr;
        std::atomic<float>* vocDeessR = nullptr;
        std::atomic<float>* vocPresG = nullptr;
        std::atomic<float>* vocAirG = nullptr;
        std::atomic<float>* vocSendRev = nullptr;

        std::atomic<float>* satOn = nullptr;
        std::atomic<float>* satType = nullptr;
        std::atomic<float>* satDrive = nullptr;
        std::atomic<float>* satTone = nullptr;
        std::atomic<float>* satHf = nullptr;
        std::atomic<float>* satMix = nullptr;

        std::atomic<float>* revOn = nullptr;
        std::atomic<float>* revSize = nullptr;
        std::atomic<float>* revDecay = nullptr;
        std::atomic<float>* revDamp = nullptr;
        std::atomic<float>* revPre = nullptr;
        std::atomic<float>* revMix = nullptr;
        std::atomic<float>* dlyOn = nullptr;
        std::atomic<float>* dlyTime = nullptr;
        std::atomic<float>* dlyFb = nullptr;
        std::atomic<float>* dlyFilter = nullptr;
        std::atomic<float>* dlyMix = nullptr;
        std::atomic<float>* dlyPingPong = nullptr;

        std::atomic<float>* imgOn = nullptr;
        std::atomic<float>* imgMonoF = nullptr;
        std::atomic<float>* imgWidth = nullptr;
        std::atomic<float>* imgWLow = nullptr;
        std::atomic<float>* imgWMid = nullptr;
        std::atomic<float>* imgWHigh = nullptr;
        std::atomic<float>* imgWAir = nullptr;
        std::atomic<float>* imgBalance = nullptr;

        std::atomic<float>* modOn = nullptr;
        std::atomic<float>* modRate = nullptr;
        std::atomic<float>* modDepth = nullptr;
        std::atomic<float>* modMix = nullptr;

        std::atomic<float>* trnOn = nullptr;
        std::atomic<float>* trnAtt = nullptr;
        std::atomic<float>* trnSus = nullptr;
        std::atomic<float>* trnFreq = nullptr;

        std::atomic<float>* ioInput = nullptr;
        std::atomic<float>* ioOutput = nullptr;
        std::atomic<float>* ioMix = nullptr;
        std::atomic<float>* ioMono = nullptr;
        std::atomic<float>* ioBypass = nullptr;
    };

    void cachePointers();
    void updateSnapFromParameters();
    void storeSnapshot();
    void pushScope (const juce::AudioBuffer<float>& buffer);

    Ptrs ptrs;
    mh::Snap snap;
    mh::MhChain chain;

    juce::AudioBuffer<float> dryBuffer;
    std::vector<float> presetSnapshot;

    std::atomic<int>   currentPreset { 0 };
    std::atomic<bool>  presetEdited { false };
    juce::String       presetName { "001 Modern Clarity" };

    // meters / analyser
    std::atomic<float> inputLevelDb  { -100.0f };
    std::atomic<float> outputLevelDb { -100.0f };
    std::atomic<float> grDb          { 0.0f };
    std::atomic<float> engineActivity { 0.0f };
    float inEnv = 0.0f, outEnv = 0.0f, grSmooth = 0.0f, activity = 0.0f;

    // FFT analyser (one tap before, one after the chain)
    mh::Analyzer analyserPre, analyserPost;

    std::array<float, 256> scope {};
    int scopeIndex = 0;
    int scopeDecim = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MHSignatureAudioProcessor)
};
