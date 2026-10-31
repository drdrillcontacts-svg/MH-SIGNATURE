#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <vector>
#include <algorithm>

/*==============================================================================
    MhChain — the whole ten-module signal path.

    TONE  → DYNAMICS → VOCAL → COLOUR → MODULATION → TRANSIENT
          → DELAY/REVERB → IMAGE → LIMITER → OUTPUT

    The four Signature Engine macros blend into the module parameters so the
    plugin keeps Milad Hashemi's measured signature as its starting point
    (see analysis/spec/ for the numbers behind every default).
==============================================================================*/

namespace mh
{

struct Snap
{
    float character = 68.0f, density = 62.0f, space = 70.0f, width = 38.0f, intensity = 100.0f;

    bool  eqOn_ = true;
    float hpf = 32.0f, lowF = 65.0f, lowG = 3.0f, lmF = 152.0f, lmG = -2.4f,
          hmF = 3500.0f, hmG = -3.5f, highF = 9000.0f, highG = -4.0f, tilt = -1.0f;

    bool  dynOn_ = true;
    float thr = -18.0f, ratio = 4.0f, att = 10.0f, rel = 100.0f, makeup = 3.2f, dynMix = 100.0f;
    bool  limOn_ = true;
    float ceiling = -0.3f, clip = 1.5f;

    bool  vocOn_ = true;
    float vocHpf = 95.0f, bodyG = 2.0f, vocThr = -22.0f, vocRatio = 4.0f,
          deessF = 6500.0f, deessRange = -4.5f, presG = -1.5f, airG = -6.0f, sendRev = 25.0f;

    bool  satOn_ = true;
    int   satType = 1;
    float drive = 4.5f, tone = 37.0f, hfRoll = 9500.0f, satMix = 58.0f;

    bool  revOn_ = true;
    float revSize = 62.0f, revDecay = 3.4f, revDamp = 70.0f, revPre = 26.0f, revMix = 24.0f;
    bool  dlyOn_ = true;
    float dlyTime = 375.0f, dlyFb = 36.0f, dlyFilter = 7200.0f, dlyMix = 18.0f;
    bool  pingPong = true;

    bool  imgOn_ = true;
    float monoF = 92.0f, imgWidth = 132.0f, wLow = 0.0f, wMid = 55.0f, wHigh = 85.0f,
          wAir = 120.0f, balance = 0.0f;

    bool  modOn_ = true;
    float modRate = 0.30f, modDepth = 28.0f, modMix = 16.0f;

    bool  trnOn_ = true;
    float trnAtt = 12.0f, trnSus = -8.0f;

    float inGain = -3.2f, outGain = -1.0f, ioMix = 100.0f;
    bool  mono = false, bypass = false;
};

//==============================================================================
inline float dbToGainF (float db) { return std::pow (10.0f, db * 0.05f); }
inline float gainToDbF (float g)  { return 20.0f * std::log10 (juce::jmax (g, 1.0e-9f)); }

//==============================================================================
/** Two-channel biquad helper. */
class Biq
{
public:
    void reset() { for (auto& f : f) f.reset(); }

    void set (const juce::dsp::IIR::Coefficients<float>::Ptr& c)
    {
        if (c == nullptr) return;
        for (auto& f : f) f.coefficients = c;
    }

    inline float process (int ch, float x) { return f[ch].processSample (x); }

    void process (juce::AudioBuffer<float>& b, int n)
    {
        for (int ch = 0; ch < juce::jmin (2, b.getNumChannels()); ++ch)
        {
            auto* d = b.getWritePointer (ch);
            for (int i = 0; i < n; ++i) d[i] = f[ch].processSample (d[i]);
        }
    }

private:
    juce::dsp::IIR::Filter<float> f[2];
};

//==============================================================================
/** One-pole low-pass, used for feedback damping and tone shaping. */
struct OnePole
{
    void setCutoff (double sr, float hz)
    {
        const auto x = std::exp (-2.0f * juce::MathConstants<float>::pi * hz / (float) sr);
        a = juce::jlimit (0.0f, 0.9999f, x);
    }
    inline float process (int ch, float in)
    {
        z[ch] = in * (1.0f - a) + z[ch] * a;
        return z[ch];
    }
    void reset() { z[0] = z[1] = 0.0f; }
    float a = 0.5f, z[2] { 0.0f, 0.0f };
};

//==============================================================================
class Delay
{
public:
    void prepare (double sr, int maxMs = 2200)
    {
        sampleRate = (float) sr;
        const int maxSamples = (int) (sr * maxMs / 1000.0) + 4;
        for (auto& d : lines) { d.assign ((size_t) maxSamples, 0.0f); }
        write = 0;
        damp[0].reset(); damp[1].reset();
        toneFilter[0].reset(); toneFilter[1].reset();
    }

    void reset()
    {
        for (auto& d : lines) std::fill (d.begin(), d.end(), 0.0f);
        write = 0;
    }

    inline float readLine (int ch, float delaySamples) const
    {
        const auto& d = lines[ch];
        const int size = (int) d.size();
        float rp = (float) write - delaySamples;
        while (rp < 0.0f) rp += (float) size;
        const int i0 = (int) rp;
        const float frac = rp - (float) i0;
        const int i1 = (i0 + 1) % size;
        return d[(size_t) i0] * (1.0f - frac) + d[(size_t) i1] * frac;
    }

    void setFilters (double sr, float dampHz, float toneHz)
    {
        damp[0].setCutoff (sr, dampHz);  damp[1].setCutoff (sr, dampHz);
        toneFilter[0].setCutoff (sr, toneHz); toneFilter[1].setCutoff (sr, toneHz);
    }

    /** Pure delay tap, no feedback (used for reverb pre-delay). */
    inline void tick (float l, float r, float delaySamples, float& dOutL, float& dOutR)
    {
        dOutL = readLine (0, delaySamples);
        dOutR = readLine (1, delaySamples);

        lines[0][(size_t) write] = l;
        lines[1][(size_t) write] = r;
        if (++write >= (int) lines[0].size()) write = 0;
    }

    /** Writes a stereo pair and returns the delayed stereo pair. */
    inline void tick (float l, float r, float delaySamples, float fb,
                      float& dOutL, float& dOutR, bool pingPong)
    {
        dOutL = readLine (0, delaySamples);
        dOutR = readLine (1, delaySamples);

        float fbL = damp[0].process (0, dOutL);
        float fbR = damp[1].process (1, dOutR);
        fbL = toneFilter[0].process (0, fbL);
        fbR = toneFilter[1].process (1, fbR);

        if (pingPong)
        {
            lines[0][(size_t) write] = l + fbR * fb;
            lines[1][(size_t) write] = r + fbL * fb;
        }
        else
        {
            lines[0][(size_t) write] = l + fbL * fb;
            lines[1][(size_t) write] = r + fbR * fb;
        }

        if (++write >= (int) lines[0].size()) write = 0;
    }

private:
    std::vector<float> lines[2];
    int write = 0;
    float sampleRate = 44100.0f;
    OnePole damp[2], toneFilter[2];
};

//==============================================================================
/** Simple modulated delay used for the modulation module. */
class ChorusLite
{
public:
    void prepare (double sr)
    {
        sampleRate = (float) sr;
        const int maxSamples = (int) (sr * 0.05) + 4;
        for (auto& d : lines) d.assign ((size_t) maxSamples, 0.0f);
        write = 0; phase = 0.0f;
    }

    void reset()
    {
        for (auto& d : lines) std::fill (d.begin(), d.end(), 0.0f);
        phase = 0.0f; write = 0;
    }

    inline void tick (float l, float r, float rateHz, float depthPct, float& outL, float& outR)
    {
        const float baseMs = 18.0f;
        const float swingMs = 8.0f * depthPct;
        phase += rateHz / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;

        const float lfoA = std::sin (phase * juce::MathConstants<float>::twoPi);
        const float lfoB = std::sin ((phase + 0.25f) * juce::MathConstants<float>::twoPi);

        const int size = (int) lines[0].size();
        auto readAt = [&] (const std::vector<float>& d, float ms) {
            float rp = (float) write - ms * 0.001f * sampleRate;
            while (rp < 0.0f) rp += (float) size;
            const int i0 = (int) rp;
            const float frac = rp - (float) i0;
            const int i1 = (i0 + 1) % size;
            return d[(size_t) i0] * (1.0f - frac) + d[(size_t) i1] * frac;
        };

        outL = readAt (lines[0], baseMs + swingMs * lfoA);
        outR = readAt (lines[1], baseMs + swingMs * lfoB);

        lines[0][(size_t) write] = l;
        lines[1][(size_t) write] = r;
        if (++write >= size) write = 0;
    }

private:
    std::vector<float> lines[2];
    int write = 0;
    float sampleRate = 44100.0f, phase = 0.0f;
};

//==============================================================================
class MhChain
{
public:
    void prepare (double sr, int blockSize, int /*numChannels*/)
    {
        sampleRate = sr;
        wetBuffer.setSize (2, juce::jmax (64, blockSize), false, true, true);
        sideBuffer.setSize (1, juce::jmax (64, blockSize), false, true, true);
        delay.prepare (sr);
        chorus.prepare (sr);
        reverb.setSampleRate (sr);
        preDelay.prepare (sr, 250);
        reset();
        prepared = true;
    }

    void reset()
    {
        for (auto& b : eqBands) b.reset();
        hpf.reset();
        vocalHpf.reset();
        deess.reset();
        body.reset();
        presence.reset();
        air.reset();
        satTone.reset();
        satHf.reset();
        sideLow.reset();
        sideMid.reset();
        sideHigh.reset();
        monoLow.reset();
        sideLowHP.reset();
        monoHP.reset();
        delay.reset();
        chorus.reset();
        preDelay.reset();
        compEnv = 0.0f;
        vocEnv = 0.0f;
        deessEnv = 0.0f;
        trnFast = 0.0f;
        trnSlow = 0.0f;
        limEnv = 0.0f;
        compGr = 0.0f;
        limGr = 0.0f;
    }

    /** Gain reduction of the compressor, in dB (negative). */
    float getGainReductionDb() const { return compGr; }
    /** Gain reduction of the limiter, in dB (negative). */
    float getLimiterGainReductionDb() const { return limGr; }

    //--------------------------------------------------------------------------
    void process (juce::AudioBuffer<float>& buffer, const Snap& p)
    {
        const int n = buffer.getNumSamples();
        if (n <= 0) return;
        if (buffer.getNumChannels() < 2) { processMonoFallback (buffer, p); return; }

        auto* L = buffer.getWritePointer (0);
        auto* R = buffer.getWritePointer (1);

        const float amt = juce::jlimit (0.0f, 1.0f, p.intensity * 0.01f);

        // ---- macro-derived targets -----------------------------------------
        const float charAmt  = p.character * 0.01f;
        const float densAmt  = p.density   * 0.01f;
        const float spaceAmt = p.space     * 0.01f;
        const float widthAmt = p.width     * 0.01f;

        // ---------------------------------------------------------- input gain
        const float inG = dbToGainF (p.inGain);
        buffer.applyGain (0, n, inG);

        // ================================================================ EQ
        if (p.eqOn_)
        {
            updateEq (p, charAmt, amt);
            hpf.process (buffer, n);
            for (auto& b : eqBands) b.process (buffer, n);
            applyTilt (buffer, n, p, charAmt, amt);
        }

        // ========================================================== DYNAMICS
        if (p.dynOn_)
            processCompressor (L, R, n, p, densAmt, amt);

        // ============================================================= VOCAL
        if (p.vocOn_)
            processVocal (buffer, n, p, amt);

        // ==================================================== COLOUR / SATURATION
        if (p.satOn_)
            processSaturation (buffer, n, p, densAmt, charAmt, amt);

        // ========================================================== MODULATION
        if (p.modOn_)
        {
            const float m = juce::jlimit (0.0f, 1.0f, p.modMix * 0.01f) * amt;
            for (int i = 0; i < n; ++i)
            {
                float wl = 0.0f, wr = 0.0f;
                chorus.tick (L[i], R[i], p.modRate, juce::jlimit (0.0f, 1.0f, p.modDepth * 0.01f), wl, wr);
                L[i] += (wl - L[i]) * m;
                R[i] += (wr - R[i]) * m;
            }
        }

        // ==================================================== TRANSIENT SHAPER
        if (p.trnOn_)
            processTransient (L, R, n, p, amt);

        // ======================================================== DELAY / REVERB
        if (p.dlyOn_)
            processDelay (L, R, n, p, spaceAmt, amt);

        if (p.revOn_)
            processReverb (L, R, n, p, spaceAmt, amt);

        // ============================================================== IMAGE
        if (p.imgOn_)
            processImager (L, R, n, p, widthAmt, amt);

        // ============================================================ LIMITER
        if (p.limOn_)
            processLimiter (L, R, n, p, densAmt, amt);

        // --------------------------------------------------------- output gain
        buffer.applyGain (0, n, dbToGainF (p.outGain));

        if (p.mono)
            for (int i = 0; i < n; ++i) { const float m = 0.5f * (L[i] + R[i]); L[i] = m; R[i] = m; }
    }

private:
    //--------------------------------------------------------------------------
    void processMonoFallback (juce::AudioBuffer<float>& buffer, const Snap& p)
    {
        juce::AudioBuffer<float> tmp (2, buffer.getNumSamples());
        tmp.copyFrom (0, 0, buffer, 0, 0, buffer.getNumSamples());
        tmp.copyFrom (1, 0, buffer, 0, 0, buffer.getNumSamples());
        process (tmp, p);
        buffer.copyFrom (0, 0, tmp, 0, 0, buffer.getNumSamples());
    }

    //--------------------------------------------------------------------------
    void updateEq (const Snap& p, float charAmt, float amt)
    {
        auto co = [&] (float f, float q, float gdb) {
            return juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                sampleRate, juce::jlimit (20.0f, (float) sampleRate * 0.45f, f), q,
                juce::Decibels::decibelsToGain (gdb * amt));
        };

        hpf.set (juce::dsp::IIR::Coefficients<float>::makeHighPass (
            sampleRate, juce::jlimit (20.0f, (float) sampleRate * 0.45f, p.hpf), 0.707f));

        eqBands[0].set (juce::dsp::IIR::Coefficients<float>::makeLowShelf (
            sampleRate, juce::jlimit (20.0f, 400.0f, p.lowF), 0.7f,
            juce::Decibels::decibelsToGain ((p.lowG + 4.0f * charAmt) * amt)));

        eqBands[1].set (co (p.lmF, 1.1f, p.lmG));
        eqBands[2].set (co (p.hmF, 0.75f, p.hmG - 2.0f * charAmt));
        eqBands[3].set (juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, juce::jlimit (2000.0f, (float) sampleRate * 0.45f, p.highF), 0.7f,
            juce::Decibels::decibelsToGain (p.highG * amt)));
    }

    void applyTilt (juce::AudioBuffer<float>& buffer, int n, const Snap& p,
                    float charAmt, float amt)
    {
        // very gentle low/high tilt pair driven by the Tilt control + Character
        const float tiltDb = juce::jlimit (-9.0f, 9.0f, (p.tilt - 1.5f * charAmt) * amt);
        if (std::abs (tiltDb) < 0.05f) return;

        auto lo = juce::dsp::IIR::Coefficients<float>::makeLowShelf (
            sampleRate, 250.0f, 0.6f, juce::Decibels::decibelsToGain (tiltDb * 0.5f));
        auto hi = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, 4000.0f, 0.6f, juce::Decibels::decibelsToGain (-tiltDb * 0.5f));
        tiltLo.set (lo); tiltHi.set (hi);
        tiltLo.process (buffer, n);
        tiltHi.process (buffer, n);
    }

    //--------------------------------------------------------------------------
    void processCompressor (float* L, float* R, int n, const Snap& p,
                            float densAmt, float amt)
    {
        const float thr = p.thr - 6.0f * densAmt;
        const float ratio = juce::jmax (1.0f, p.ratio + 4.0f * densAmt);
        const float attMs = p.att, relMs = p.rel;
        const float aA = std::exp (-1.0f / (0.001f * attMs * (float) sampleRate));
        const float aR = std::exp (-1.0f / (0.001f * relMs * (float) sampleRate));
        const float makeupG = dbToGainF (p.makeup);
        const float mix = juce::jlimit (0.0f, 1.0f, p.dynMix * 0.01f);

        float maxGr = 0.0f;
        for (int i = 0; i < n; ++i)
        {
            const float peak = juce::jmax (std::abs (L[i]), std::abs (R[i]));
            const float db = gainToDbF (peak);
            const float over = db - thr;
            const float targetGrDb = over > 0.0f ? -over * (1.0f - 1.0f / ratio) : 0.0f;

            const float coeff = (targetGrDb < -0.0001f) ? aA : aR;
            compEnv = compEnv * coeff + targetGrDb * (1.0f - coeff);
            if (compEnv < -48.0f) compEnv = -48.0f;
            maxGr = juce::jmin (maxGr, compEnv);

            const float g = dbToGainF (compEnv) * makeupG;
            const float gainApplied = 1.0f + (g - 1.0f) * mix;
            L[i] *= gainApplied;
            R[i] *= gainApplied;
        }
        compGr = maxGr;
    }

    //--------------------------------------------------------------------------
    void processVocal (juce::AudioBuffer<float>& buffer, int n, const Snap& p, float amt)
    {
        vocalHpf.set (juce::dsp::IIR::Coefficients<float>::makeHighPass (
            sampleRate, juce::jlimit (20.0f, 400.0f, p.vocHpf), 0.707f));
        vocalHpf.process (buffer, n);

        body.set (juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            sampleRate, 190.0f, 1.0f, juce::Decibels::decibelsToGain (p.bodyG * amt)));
        body.process (buffer, n);

        presence.set (juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            sampleRate, 3200.0f, 0.8f, juce::Decibels::decibelsToGain (p.presG * amt)));
        presence.process (buffer, n);

        air.set (juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, juce::jlimit (4000.0f, (float) sampleRate * 0.45f, 12000.0f), 0.7f,
            juce::Decibels::decibelsToGain (p.airG * amt)));
        air.process (buffer, n);

        // ---- de-esser: band-limited dynamic gain reduction -----------------
        deess.set (juce::dsp::IIR::Coefficients<float>::makeBandPass (
            sampleRate, juce::jlimit (2000.0f, (float) sampleRate * 0.45f, p.deessF), 1.6f));

        const float range = juce::jlimit (-18.0f, 0.0f, p.deessRange);
        const float att = std::exp (-1.0f / (0.001f * 2.0f * (float) sampleRate));
        const float rel = std::exp (-1.0f / (0.001f * 60.0f * (float) sampleRate));

        for (int ch = 0; ch < juce::jmin (2, buffer.getNumChannels()); ++ch)
        {
            auto* d = buffer.getWritePointer (ch);
            for (int i = 0; i < n; ++i)
            {
                const float band = deess.process (ch, d[i]);
                const float lvl = std::abs (band);
                const float c = (lvl > deessEnv) ? att : rel;
                deessEnv = deessEnv * c + lvl * (1.0f - c);
                const float over = gainToDbF (deessEnv) + 40.0f;  // -40 dB reference
                if (over > 0.0f)
                {
                    const float grDb = juce::jlimit (range, 0.0f, -over * 0.35f);
                    d[i] *= dbToGainF (grDb * amt);
                }
            }
        }

        // ---- vocal compression --------------------------------------------
        const float aA = std::exp (-1.0f / (0.001f * 3.0f * (float) sampleRate));
        const float aR = std::exp (-1.0f / (0.001f * 70.0f * (float) sampleRate));
        const float ratio = juce::jmax (1.0f, p.vocRatio);

        for (int ch = 0; ch < juce::jmin (2, buffer.getNumChannels()); ++ch)
        {
            auto* d = buffer.getWritePointer (ch);
            for (int i = 0; i < n; ++i)
            {
                const float db = gainToDbF (std::abs (d[i]));
                const float over = db - p.vocThr;
                const float target = over > 0.0f ? -over * (1.0f - 1.0f / ratio) : 0.0f;
                const float c = (target < vocEnv) ? aA : aR;
                vocEnv = vocEnv * c + target * (1.0f - c);
                d[i] *= dbToGainF (vocEnv * amt);
            }
        }
    }

    //--------------------------------------------------------------------------
    void processSaturation (juce::AudioBuffer<float>& buffer, int n, const Snap& p,
                            float densAmt, float charAmt, float amt)
    {
        const float driveDb = p.drive + 4.0f * densAmt;
        const float pre = dbToGainF (driveDb);
        const float mix = juce::jlimit (0.0f, 1.0f, p.satMix * 0.01f) * amt;
        const float typeGain = 1.0f / juce::jmax (1.0f, pre * 0.6f);

        for (int ch = 0; ch < juce::jmin (2, buffer.getNumChannels()); ++ch)
        {
            auto* d = buffer.getWritePointer (ch);
            for (int i = 0; i < n; ++i)
            {
                const float dry = d[i];
                const float x = dry * pre;
                float wet = 0.0f;

                switch (p.satType)
                {
                    case 0:  wet = std::tanh (x * 0.8f); break;                      // tape
                    case 1:  wet = std::tanh (x * 1.1f) * 0.92f; break;              // console
                    case 2:  wet = (x - x * x * x * 0.15f);                          // tube-ish
                             wet = juce::jlimit (-1.5f, 1.5f, wet); break;
                    case 3:  wet = std::tanh (x) * 1.05f; break;                     // diode
                    default: wet = juce::jlimit (-1.0f, 1.0f, x); break;             // hard clip
                }

                wet *= typeGain;
                d[i] = dry + (wet - dry) * mix;
            }
        }

        const float toneDb = (p.tone - 50.0f) * 0.08f - 1.0f * charAmt;
        satTone.set (juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, 3000.0f, 0.7f, juce::Decibels::decibelsToGain (toneDb * amt)));
        satTone.process (buffer, n);

        satHf.set (juce::dsp::IIR::Coefficients<float>::makeLowPass (
            sampleRate, juce::jlimit (2000.0f, (float) sampleRate * 0.45f, p.hfRoll), 0.707f));
        satHf.process (buffer, n);
    }

    //--------------------------------------------------------------------------
    void processTransient (float* L, float* R, int n, const Snap& p, float amt)
    {
        const float attDb = p.trnAtt * 0.06f;
        const float susDb = p.trnSus * 0.06f;
        const float fastA = std::exp (-1.0f / (0.0005f * (float) sampleRate));
        const float slowA = std::exp (-1.0f / (0.020f  * (float) sampleRate));
        const float mixAmt = amt;

        for (int i = 0; i < n; ++i)
        {
            const float peak = juce::jmax (std::abs (L[i]), std::abs (R[i]));
            trnFast = trnFast * fastA + peak * (1.0f - fastA);
            trnSlow = trnSlow * slowA + peak * (1.0f - slowA);

            const float diff = gainToDbF (trnFast + 1.0e-6f) - gainToDbF (trnSlow + 1.0e-6f);
            const float g = dbToGainF ((attDb * juce::jlimit (0.0f, 3.0f, diff) * 0.33f
                                      + susDb * juce::jlimit (-1.0f, 0.0f, -diff * 0.05f)) * mixAmt);
            L[i] *= g;
            R[i] *= g;
        }
    }

    //--------------------------------------------------------------------------
    void processDelay (float* L, float* R, int n, const Snap& p, float spaceAmt, float amt)
    {
        const float delaySamples = juce::jlimit (1.0f, (float) (sampleRate * 2.0),
                                                 p.dlyTime * 0.001f * (float) sampleRate);
        const float fb = juce::jlimit (0.0f, 0.95f, p.dlyFb * 0.01f);
        const float mix = juce::jlimit (0.0f, 1.0f, p.dlyMix * 0.01f) * amt;
        delay.setFilters (sampleRate, juce::jlimit (500.0f, 18000.0f, p.dlyFilter), p.dlyFilter);

        for (int i = 0; i < n; ++i)
        {
            float dl = 0.0f, dr = 0.0f;
            delay.tick (L[i], R[i], delaySamples, fb, dl, dr, p.pingPong);
            L[i] += dl * mix;
            R[i] += dr * mix;
        }
    }

    //--------------------------------------------------------------------------
    void processReverb (float* L, float* R, int n, const Snap& p, float spaceAmt, float amt)
    {
        juce::Reverb::Parameters rp;
        const float sizeNorm = juce::jlimit (0.0f, 1.0f, (p.revSize * 0.6f + p.revDecay * 3.2f) * 0.01f);
        rp.roomSize = juce::jlimit (0.0f, 1.0f, sizeNorm * (0.6f + 0.4f * spaceAmt) + 0.15f);
        rp.damping  = juce::jlimit (0.0f, 1.0f, p.revDamp * 0.01f);
        rp.width    = 1.0f;
        rp.wetLevel = 1.0f;
        rp.dryLevel = 0.0f;
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);

        const float mix = juce::jlimit (0.0f, 1.0f, p.revMix * 0.01f) * amt;

        // pre-delay into a scratch buffer (never reallocated while playing)
        if (wetBuffer.getNumSamples() < n)
            wetBuffer.setSize (2, n, false, true, true);

        juce::AudioBuffer<float> wet (wetBuffer.getArrayOfWritePointers(), 2, n);
        for (int i = 0; i < n; ++i)
        {
            float a = 0.0f, b = 0.0f;
            preDelay.tick (L[i], R[i], juce::jlimit (1.0f, 5000.0f, p.revPre * 0.001f * (float) sampleRate), a, b);
            wet.setSample (0, i, a);
            wet.setSample (1, i, b);
        }

        reverb.processStereo (wet.getWritePointer (0), wet.getWritePointer (1), n);

        for (int i = 0; i < n; ++i)
        {
            L[i] += wet.getSample (0, i) * mix;
            R[i] += wet.getSample (1, i) * mix;
        }
    }

    //--------------------------------------------------------------------------
    void processImager (float* L, float* R, int n, const Snap& p, float widthAmt, float amt)
    {
        const float globalW = juce::jlimit (0.0f, 2.0f, (p.imgWidth * 0.01f) * (0.5f + widthAmt));
        const float bal = juce::jlimit (-1.0f, 1.0f, p.balance * 0.01f);

        // frequency dependent width via the side channel only
        const float lowDb  = (p.wLow  - 55.0f) * 0.05f;
        const float midDb  = (p.wMid  - 85.0f) * 0.05f;
        const float airDb  = (p.wAir  - 100.0f) * 0.05f;

        sideLow.set (juce::dsp::IIR::Coefficients<float>::makeLowShelf (
            sampleRate, 200.0f, 0.7f, juce::Decibels::decibelsToGain (lowDb * amt)));
        sideMid.set (juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            sampleRate, 1500.0f, 1.2f, juce::Decibels::decibelsToGain (midDb * amt)));
        sideHigh.set (juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, 7000.0f, 0.7f, juce::Decibels::decibelsToGain (airDb * amt)));

        monoLow.set (juce::dsp::IIR::Coefficients<float>::makeLowPass (
            sampleRate, juce::jlimit (30.0f, 400.0f, p.monoF), 0.707f));

        if (sideBuffer.getNumSamples() < n)
            sideBuffer.setSize (1, n, false, true, true);

        juce::AudioBuffer<float> side (sideBuffer.getArrayOfWritePointers(), 1, n);
        auto* sideD = side.getWritePointer (0);

        for (int i = 0; i < n; ++i)
        {
            const float m = 0.5f * (L[i] + R[i]);
            sideD[i] = 0.5f * (L[i] - R[i]);

            // mono-maker on the low band of the side signal
            const float lowSide = monoLow.process (0, sideD[i]);
            sideD[i] -= lowSide * amt;
        }

        sideLow.process (side, n);
        sideMid.process (side, n);
        sideHigh.process (side, n);

        for (int i = 0; i < n; ++i)
        {
            const float m = 0.5f * (L[i] + R[i]);
            const float s = sideD[i] * globalW;
            L[i] = (m + s) * (1.0f - juce::jmax (0.0f, bal));
            R[i] = (m - s) * (1.0f + juce::jmin (0.0f, bal));
        }
    }

    //--------------------------------------------------------------------------
    void processLimiter (float* L, float* R, int n, const Snap& p, float densAmt, float amt)
    {
        const float ceil = p.ceiling;
        const float clipDb = p.clip * (0.5f + densAmt);
        const float rel = std::exp (-1.0f / (0.150f * (float) sampleRate));

        const float ceilGain = dbToGainF (ceil);
        const float knee = ceilGain * 0.62f;      // everything under the knee passes untouched
        const float span = juce::jmax (1.0e-4f, ceilGain - knee);

        auto softClip = [knee, span] (float x)
        {
            const float ax = std::abs (x);
            if (ax <= knee) return x;
            const float shaped = knee + span * std::tanh ((ax - knee) / span);
            return x < 0.0f ? -shaped : shaped;
        };

        float maxGr = limGr;
        for (int i = 0; i < n; ++i)
        {
            float peak = juce::jmax (std::abs (L[i]), std::abs (R[i]));
            if (peak > 1.0e-6f)
            {
                const float peakDb = gainToDbF (peak) + clipDb;
                const float over = peakDb - ceil;
                const float targetGr = over > 0.0f ? -over : 0.0f;
                if (targetGr < limEnv) limEnv = targetGr;
                else limEnv = limEnv * rel + targetGr * (1.0f - rel);
                maxGr = juce::jmin (maxGr, limEnv);
                const float g = dbToGainF (limEnv * amt);
                L[i] *= g;
                R[i] *= g;
            }

            // soft clip only where it is actually needed
            L[i] = softClip (L[i]);
            R[i] = softClip (R[i]);
        }
        limGr = maxGr * 0.98f;
    }

    //--------------------------------------------------------------------------
    double sampleRate = 44100.0;
    bool prepared = false;

    Biq hpf, tiltLo, tiltHi;
    Biq eqBands[4];

    Biq vocalHpf, body, presence, air, deess;

    Biq satTone, satHf;

    Biq sideLow, sideMid, sideHigh, monoLow, sideLowHP, monoHP;

    Delay delay, preDelay;
    juce::AudioBuffer<float> wetBuffer, sideBuffer;
    ChorusLite chorus;
    juce::Reverb reverb;

    float compEnv = 0.0f, vocEnv = 0.0f, deessEnv = 0.0f;
    float trnFast = 0.0f, trnSlow = 0.0f;
    float limEnv = 0.0f;
    float compGr = 0.0f;
    float limGr = 0.0f;
};

} // namespace mh
