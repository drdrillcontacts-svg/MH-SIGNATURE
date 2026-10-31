#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MHSignatureAudioProcessor::MHSignatureAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MHSIGNATURE", createLayout())
{
    cachePointers();
    updateSnapFromParameters();
    presetName = getFactoryPresetName (0);
    currentPreset = 0;
    storeSnapshot();
}

MHSignatureAudioProcessor::~MHSignatureAudioProcessor() = default;

//==============================================================================
void MHSignatureAudioProcessor::cachePointers()
{
    auto gp = [this] (const char* id) { return apvts.getRawParameterValue (id); };

    ptrs.character = gp (Params::character);
    ptrs.density   = gp (Params::density);
    ptrs.space     = gp (Params::space);
    ptrs.width     = gp (Params::width);
    ptrs.intensity = gp (Params::intensity);

    ptrs.eqOn    = gp (Params::eqOn);
    ptrs.eqHpf   = gp (Params::eqHpf);
    ptrs.eqLowF  = gp (Params::eqLowF);
    ptrs.eqLowG  = gp (Params::eqLowG);
    ptrs.eqLmF   = gp (Params::eqLmF);
    ptrs.eqLmG   = gp (Params::eqLmG);
    ptrs.eqHmF   = gp (Params::eqHmF);
    ptrs.eqHmG   = gp (Params::eqHmG);
    ptrs.eqHighF = gp (Params::eqHighF);
    ptrs.eqHighG = gp (Params::eqHighG);
    ptrs.eqTilt  = gp (Params::eqTilt);

    ptrs.dynOn     = gp (Params::dynOn);
    ptrs.dynThr    = gp (Params::dynThr);
    ptrs.dynRatio  = gp (Params::dynRatio);
    ptrs.dynAtt    = gp (Params::dynAtt);
    ptrs.dynRel    = gp (Params::dynRel);
    ptrs.dynMakeup = gp (Params::dynMakeup);
    ptrs.dynMix    = gp (Params::dynMix);
    ptrs.limOn     = gp (Params::limOn);
    ptrs.limCeiling= gp (Params::limCeiling);
    ptrs.limClip   = gp (Params::limClip);

    ptrs.vocOn     = gp (Params::vocOn);
    ptrs.vocHpf    = gp (Params::vocHpf);
    ptrs.vocBodyG  = gp (Params::vocBodyG);
    ptrs.vocThr    = gp (Params::vocThr);
    ptrs.vocRatio  = gp (Params::vocRatio);
    ptrs.vocDeessF = gp (Params::vocDeessF);
    ptrs.vocDeessR = gp (Params::vocDeessR);
    ptrs.vocPresG  = gp (Params::vocPresG);
    ptrs.vocAirG   = gp (Params::vocAirG);
    ptrs.vocSendRev= gp (Params::vocSendRev);

    ptrs.satOn    = gp (Params::satOn);
    ptrs.satType  = gp (Params::satType);
    ptrs.satDrive = gp (Params::satDrive);
    ptrs.satTone  = gp (Params::satTone);
    ptrs.satHf    = gp (Params::satHf);
    ptrs.satMix   = gp (Params::satMix);

    ptrs.revOn    = gp (Params::revOn);
    ptrs.revSize  = gp (Params::revSize);
    ptrs.revDecay = gp (Params::revDecay);
    ptrs.revDamp  = gp (Params::revDamp);
    ptrs.revPre   = gp (Params::revPre);
    ptrs.revMix   = gp (Params::revMix);
    ptrs.dlyOn    = gp (Params::dlyOn);
    ptrs.dlyTime  = gp (Params::dlyTime);
    ptrs.dlyFb    = gp (Params::dlyFb);
    ptrs.dlyFilter= gp (Params::dlyFilter);
    ptrs.dlyMix   = gp (Params::dlyMix);
    ptrs.dlyPingPong = gp (Params::dlyPingPong);

    ptrs.imgOn      = gp (Params::imgOn);
    ptrs.imgMonoF   = gp (Params::imgMonoF);
    ptrs.imgWidth   = gp (Params::imgWidth);
    ptrs.imgWLow    = gp (Params::imgWLow);
    ptrs.imgWMid    = gp (Params::imgWMid);
    ptrs.imgWHigh   = gp (Params::imgWHigh);
    ptrs.imgWAir    = gp (Params::imgWAir);
    ptrs.imgBalance = gp (Params::imgBalance);

    ptrs.modOn    = gp (Params::modOn);
    ptrs.modRate  = gp (Params::modRate);
    ptrs.modDepth = gp (Params::modDepth);
    ptrs.modMix   = gp (Params::modMix);

    ptrs.trnOn  = gp (Params::trnOn);
    ptrs.trnAtt = gp (Params::trnAtt);
    ptrs.trnSus = gp (Params::trnSus);
    ptrs.trnFreq= gp (Params::trnFreq);

    ptrs.ioInput  = gp (Params::ioInput);
    ptrs.ioOutput = gp (Params::ioOutput);
    ptrs.ioMix    = gp (Params::ioMix);
    ptrs.ioMono   = gp (Params::ioMono);
    ptrs.ioBypass = gp (Params::ioBypass);
}

//==============================================================================
void MHSignatureAudioProcessor::updateSnapFromParameters()
{
    auto f = [] (std::atomic<float>* p) { return p != nullptr ? p->load() : 0.0f; };
    auto b = [] (std::atomic<float>* p) { return p != nullptr && p->load() > 0.5f; };

    snap.character = f (ptrs.character);
    snap.density   = f (ptrs.density);
    snap.space     = f (ptrs.space);
    snap.width     = f (ptrs.width);
    snap.intensity = f (ptrs.intensity);

    snap.eqOn_ = b (ptrs.eqOn);
    snap.hpf   = f (ptrs.eqHpf);
    snap.lowF  = f (ptrs.eqLowF);
    snap.lowG  = f (ptrs.eqLowG);
    snap.lmF   = f (ptrs.eqLmF);
    snap.lmG   = f (ptrs.eqLmG);
    snap.hmF   = f (ptrs.eqHmF);
    snap.hmG   = f (ptrs.eqHmG);
    snap.highF = f (ptrs.eqHighF);
    snap.highG = f (ptrs.eqHighG);
    snap.tilt  = f (ptrs.eqTilt);

    snap.dynOn_  = b (ptrs.dynOn);
    snap.thr     = f (ptrs.dynThr);
    snap.ratio   = f (ptrs.dynRatio);
    snap.att     = f (ptrs.dynAtt);
    snap.rel     = f (ptrs.dynRel);
    snap.makeup  = f (ptrs.dynMakeup);
    snap.dynMix  = f (ptrs.dynMix);
    snap.limOn_  = b (ptrs.limOn);
    snap.ceiling = f (ptrs.limCeiling);
    snap.clip    = f (ptrs.limClip);

    snap.vocOn_     = b (ptrs.vocOn);
    snap.vocHpf     = f (ptrs.vocHpf);
    snap.bodyG      = f (ptrs.vocBodyG);
    snap.vocThr     = f (ptrs.vocThr);
    snap.vocRatio   = f (ptrs.vocRatio);
    snap.deessF     = f (ptrs.vocDeessF);
    snap.deessRange = f (ptrs.vocDeessR);
    snap.presG      = f (ptrs.vocPresG);
    snap.airG       = f (ptrs.vocAirG);
    snap.sendRev    = f (ptrs.vocSendRev);

    snap.satOn_  = b (ptrs.satOn);
    snap.satType = (int) f (ptrs.satType);
    snap.drive   = f (ptrs.satDrive);
    snap.tone    = f (ptrs.satTone);
    snap.hfRoll  = f (ptrs.satHf);
    snap.satMix  = f (ptrs.satMix);

    snap.revOn_   = b (ptrs.revOn);
    snap.revSize  = f (ptrs.revSize);
    snap.revDecay = f (ptrs.revDecay);
    snap.revDamp  = f (ptrs.revDamp);
    snap.revPre   = f (ptrs.revPre);
    snap.revMix   = f (ptrs.revMix);
    snap.dlyOn_   = b (ptrs.dlyOn);
    snap.dlyTime  = f (ptrs.dlyTime);
    snap.dlyFb    = f (ptrs.dlyFb);
    snap.dlyFilter= f (ptrs.dlyFilter);
    snap.dlyMix   = f (ptrs.dlyMix);
    snap.pingPong = b (ptrs.dlyPingPong);

    snap.imgOn_   = b (ptrs.imgOn);
    snap.monoF    = f (ptrs.imgMonoF);
    snap.imgWidth = f (ptrs.imgWidth);
    snap.wLow     = f (ptrs.imgWLow);
    snap.wMid     = f (ptrs.imgWMid);
    snap.wHigh    = f (ptrs.imgWHigh);
    snap.wAir     = f (ptrs.imgWAir);
    snap.balance  = f (ptrs.imgBalance);

    snap.modOn_   = b (ptrs.modOn);
    snap.modRate  = f (ptrs.modRate);
    snap.modDepth = f (ptrs.modDepth);
    snap.modMix   = f (ptrs.modMix);

    snap.trnOn_ = b (ptrs.trnOn);
    snap.trnAtt = f (ptrs.trnAtt);
    snap.trnSus = f (ptrs.trnSus);

    snap.inGain  = f (ptrs.ioInput);
    snap.outGain = f (ptrs.ioOutput);
    snap.ioMix   = f (ptrs.ioMix);
    snap.mono    = b (ptrs.ioMono);
    snap.bypass  = b (ptrs.ioBypass);
}

//==============================================================================
void MHSignatureAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);

    chain.prepare (sampleRate, samplesPerBlock, juce::jmax (1, getTotalNumOutputChannels()));
    dryBuffer.setSize (2, juce::jmax (1, samplesPerBlock), false, true, true);

    analyserPre.prepare (sampleRate);
    analyserPost.prepare (sampleRate);

    scope.fill (0.0f);
    scopeIndex = 0;
    scopeDecim = 0;

    resetMeters();
}

void MHSignatureAudioProcessor::releaseResources()
{
    chain.reset();
}

bool MHSignatureAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    const auto& in  = layouts.getMainInputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    if (in != out)
        return false;

    return true;
}

void MHSignatureAudioProcessor::resetMeters()
{
    inputLevelDb.store (-100.0f);
    outputLevelDb.store (-100.0f);
    grDb.store (0.0f);
    engineActivity.store (0.0f);
    inEnv = outEnv = grSmooth = activity = 0.0f;
}

//==============================================================================
void MHSignatureAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused (midiMessages);

    const int numSamples = buffer.getNumSamples();
    const int numCh      = buffer.getNumChannels();

    for (int ch = getTotalNumInputChannels(); ch < numCh; ++ch)
        buffer.clear (ch, 0, numSamples);

    if (numCh == 0 || numSamples == 0)
        return;

    updateSnapFromParameters();

    // ---- input meter (pre-processing) -----------------------------------
    {
        float peak = 0.0f;
        for (int ch = 0; ch < numCh; ++ch)
            peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, numSamples));
        inEnv = juce::jmax (peak, inEnv * 0.82f);
        inputLevelDb.store (juce::Decibels::gainToDecibels (inEnv, -100.0f));
    }

    analyserPre.push (buffer);

    const bool bypass = snap.bypass;
    const float mix = juce::jlimit (0.0f, 1.0f, snap.ioMix * 0.01f);

    if (! bypass)
    {
        if (mix < 0.999f && numCh >= 2)
        {
            if (dryBuffer.getNumSamples() < numSamples)
                dryBuffer.setSize (2, numSamples, false, true, true);
            dryBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
            dryBuffer.copyFrom (1, 0, buffer, juce::jmin (1, numCh - 1), 0, numSamples);
        }

        chain.process (buffer, snap);

        if (mix < 0.999f && numCh >= 2)
        {
            for (int ch = 0; ch < 2; ++ch)
            {
                buffer.applyGain (ch, 0, numSamples, 1.0f - mix);
                buffer.addFrom (ch, 0, dryBuffer, ch, 0, numSamples, mix);
            }
        }
    }

    // ---- output meter ----------------------------------------------------
    {
        float peak = 0.0f;
        for (int ch = 0; ch < numCh; ++ch)
            peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, numSamples));
        outEnv = juce::jmax (peak, outEnv * 0.82f);
        outputLevelDb.store (juce::Decibels::gainToDecibels (outEnv, -100.0f));

        const float targetGr = chain.getGainReductionDb();
        grSmooth += (targetGr - grSmooth) * 0.25f;
        grDb.store (grSmooth);

        activity += (juce::jlimit (0.0f, 1.0f, outEnv * 1.2f) - activity) * 0.12f;
        engineActivity.store (activity);
    }

    analyserPost.push (buffer);
    pushScope (buffer);
}

//==============================================================================
void MHSignatureAudioProcessor::pushScope (const juce::AudioBuffer<float>& buffer)
{
    const int n = buffer.getNumSamples();
    const int chans = buffer.getNumChannels();
    if (n <= 0 || chans <= 0) return;

    const auto* l = buffer.getReadPointer (0);
    const auto* r = chans > 1 ? buffer.getReadPointer (1) : l;

    // roughly 3 ms of signal per slot, whatever the block size
    const int step = juce::jmax (1, (int) (getSampleRate() / 44100.0 * 96.0));

    for (int i = 0; i < n; ++i)
    {
        if (++scopeDecim >= step)
        {
            scopeDecim = 0;
            scope[(size_t) scopeIndex] = 0.5f * (l[i] + r[i]);
            scopeIndex = (scopeIndex + 1) % (int) scope.size();
        }
    }
}

void MHSignatureAudioProcessor::getSpectrum (float* dest, bool pre) const
{
    if (pre) analyserPre.copyBands (dest);
    else     analyserPost.copyBands (dest);
}

void MHSignatureAudioProcessor::getWaveform (float* dest) const
{
    if (dest == nullptr) return;

    const int n = (int) scope.size();
    for (int i = 0; i < n; ++i)
        dest[i] = scope[(size_t) ((scopeIndex + i) % n)];
}

//==============================================================================
int MHSignatureAudioProcessor::getNumPrograms()
{
    return juce::jmax (1, getNumFactoryPresets());
}

juce::String MHSignatureAudioProcessor::getFactoryPresetName (int index)
{
    const auto& presets = getFactoryPresets();
    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return "INIT";
    return presets[(size_t) index].name;
}

juce::String MHSignatureAudioProcessor::getFactoryPresetCategory (int index)
{
    const auto& presets = getFactoryPresets();
    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return "Signature";
    return presets[(size_t) index].category;
}

const juce::String MHSignatureAudioProcessor::getProgramName (int index)
{
    return getFactoryPresetName (index);
}

void MHSignatureAudioProcessor::setCurrentProgram (int index)
{
    applyPreset (index);
}

void MHSignatureAudioProcessor::applyPreset (int index)
{
    const auto& presets = getFactoryPresets();
    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return;

    const auto& p = presets[(size_t) index];

    // every knob the preset does not mention falls back to its factory default,
    // so switching presets never leaves a value behind from the previous one
    const auto& params = getParameters();
    for (int i = 0; i < params.size(); ++i)
    {
        if (auto* param = params.getUnchecked (i))
            param->setValueNotifyingHost (param->getDefaultValue());
    }

    for (const auto& kv : p.values)
    {
        if (auto* param = apvts.getParameter (kv.first))
        {
            const float v = param->getNormalisableRange().snapToLegalValue (kv.second);
            param->setValueNotifyingHost (param->convertTo0to1 (v));
        }
    }

    currentPreset = index;
    presetName = p.name;
    presetEdited = false;
    updateSnapFromParameters();
    storeSnapshot();
}

void MHSignatureAudioProcessor::storeSnapshot()
{
    const auto& params = getParameters();
    presetSnapshot.assign ((size_t) params.size(), 0.0f);
    for (int i = 0; i < params.size(); ++i)
        presetSnapshot[(size_t) i] = params.getUnchecked (i)->getValue();
    presetEdited = false;
}

bool MHSignatureAudioProcessor::isPresetEdited() const
{
    const auto& params = getParameters();
    if ((int) presetSnapshot.size() != params.size())
        return false;

    for (int i = 0; i < params.size(); ++i)
        if (std::abs (params.getUnchecked (i)->getValue()
                      - presetSnapshot[(size_t) i]) > 1.0e-4f)
            return true;

    return false;
}

//==============================================================================
void MHSignatureAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("presetIndex", currentPreset.load(), nullptr);
    state.setProperty ("presetName", presetName, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void MHSignatureAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr) return;

    if (! xml->hasTagName (apvts.state.getType()))
        return;

    const int presetIndex = (int) xml->getIntAttribute ("presetIndex", 0);

    apvts.replaceState (juce::ValueTree::fromXml (*xml));

    currentPreset = juce::jlimit (0, juce::jmax (0, getNumFactoryPresets() - 1), presetIndex);
    presetName = xml->getStringAttribute ("presetName", getFactoryPresetName (currentPreset));

    updateSnapFromParameters();
    storeSnapshot();
}

//==============================================================================
juce::AudioProcessorEditor* MHSignatureAudioProcessor::createEditor()
{
    return new MHSignatureAudioProcessorEditor (*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MHSignatureAudioProcessor();
}
