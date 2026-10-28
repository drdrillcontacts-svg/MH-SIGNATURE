#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace mh
{

/*==============================================================================
    Log-spaced 96-band analyser feeding the editor's spectrum display.
    One instance per tap (pre / post).
==============================================================================*/
class Analyzer
{
public:
    static constexpr int numBands = 96;
    static constexpr int fftOrder = 10;
    static constexpr int fftSize  = 1 << fftOrder;

    Analyzer()
    {
        for (auto& b : smooth) b = -90.0f;
    }

    void prepare (double sr)
    {
        sampleRate = (float) juce::jmax (8000.0, sr);
        reset();
    }

    void reset()
    {
        fifo.fill (0.0f);
        data.fill (0.0f);
        for (auto& b : smooth) b = -90.0f;
        index = 0;
        ready = false;
    }

    void push (const juce::AudioBuffer<float>& buffer)
    {
        const int n = buffer.getNumSamples();
        const int chans = buffer.getNumChannels();
        if (n <= 0 || chans <= 0) return;

        const auto* l = buffer.getReadPointer (0);
        const auto* r = chans > 1 ? buffer.getReadPointer (1) : l;

        for (int i = 0; i < n; ++i)
        {
            fifo[(size_t) index] = 0.5f * (l[i] + r[i]);
            if (++index >= fftSize)
            {
                index = 0;
                ready = true;
                break;
            }
        }

        if (! ready) return;
        ready = false;

        std::copy (fifo.begin(), fifo.end(), data.begin());
        window.multiplyWithWindowingTable (data.data(), (size_t) fftSize);
        fft.performFrequencyOnlyForwardTransform (data.data());

        const float binHz = sampleRate / (float) fftSize;

        auto freqToBin = [&] (float hz)
        {
            return (int) juce::jlimit (0.0f, (float) (fftSize / 2 - 1), hz / binHz);
        };

        for (int i = 0; i < numBands; ++i)
        {
            const float f0 = 20.0f * std::pow (1000.0f, (float) i / (float) numBands);
            const float f1 = 20.0f * std::pow (1000.0f, (float) (i + 1) / (float) numBands);

            const int b0 = freqToBin (f0);
            const int b1 = juce::jmax (b0, freqToBin (f1));

            float mag = 0.0f;
            for (int b = b0; b <= b1 && b < fftSize / 2; ++b)
                mag = juce::jmax (mag, data[(size_t) b]);

            float db = juce::Decibels::gainToDecibels (mag, -100.0f);
            db += 3.2f * std::log2 (juce::jmax (20.0f, f0) / 1000.0f);   // analyser tilt
            db = juce::jlimit (-90.0f, 6.0f, db);

            auto& s = smooth[(size_t) i];
            s += (db - s) * (db > s ? 0.6f : 0.15f);
        }
    }

    void copyBands (float* dest) const
    {
        if (dest == nullptr) return;
        for (int i = 0; i < numBands; ++i)
            dest[i] = smooth[(size_t) i];
    }

    float band (int i) const { return smooth[(size_t) juce::jlimit (0, numBands - 1, i)]; }

private:
    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize,
                                                 juce::dsp::WindowingFunction<float>::hann };
    std::array<float, fftSize> fifo {};
    std::array<float, 2 * fftSize> data {};
    std::array<float, numBands> smooth {};
    int index = 0;
    bool ready = false;
    float sampleRate = 44100.0f;
};

} // namespace mh
