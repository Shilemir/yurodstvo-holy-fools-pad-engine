// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <vector>

// Eight unequal delay lines, energy-preserving Hadamard scattering and
// frequency-dependent RT60 loss. All storage is allocated in prepare().
class CathedralReverb
{
    struct Line
    {
        std::vector<float> data;
        size_t position = 0;
        float read() const { return data[position]; }
        void write(float x) { data[position] = x; if (++position == data.size()) position = 0; }
        void prepare(int n) { data.assign(static_cast<size_t>(n), 0.0f); position = 0; }
        void clear() { std::fill(data.begin(), data.end(), 0.0f); position = 0; }
    };
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        constexpr std::array<double, 8> seconds { .0797, .0893, .1013, .1139, .1277, .1433, .1637, .1831 };
        for (size_t i = 0; i < 8; ++i)
        {
            lengths[i] = seconds[i];
            lines[i].prepare(juce::jmax(1, juce::roundToInt(sr * seconds[i]) | 1));
        }
        constexpr std::array<double, 4> diffusion { .0047, .0073, .0113, .0179 };
        for (size_t i = 0; i < 4; ++i) diffusers[i].prepare(juce::jmax(1, juce::roundToInt(sr * diffusion[i])));
        predelay.prepare(juce::jmax(1, juce::roundToInt(sr * .027)));
        lowpass = 1 - std::exp(-juce::MathConstants<float>::twoPi * 5800 / static_cast<float>(sr));
        clear();
    }
    void clear()
    {
        for (auto& l : lines) l.clear();
        for (auto& l : diffusers) l.clear();
        predelay.clear(); damped = {}; dcInput = {}; dcOutput = {}; tick = 0;
    }
    std::array<float, 2> process(std::array<float, 2> input, float amount)
    {
        // Coefficients update at control rate, using the already smoothed macro.
        if (tick++ % 64 == 0)
        {
            const float rt60 = 1.4f + 28.6f * amount * amount;
            for (size_t i = 0; i < 8; ++i)
                loss[i] = std::pow(0.001f, static_cast<float>(lengths[i]) / rt60);
        }
        float excitation = (input[0] + input[1]) * .5f;
        const float delayed = predelay.read(); predelay.write(excitation); excitation = delayed;
        for (auto& ap : diffusers)
        {
            const float tap = ap.read();
            const float y = tap - .65f * excitation;
            ap.write(excitation + .65f * y); excitation = y;
        }
        std::array<float, 8> taps {}, scatter {};
        for (size_t i = 0; i < 8; ++i)
        {
            taps[i] = lines[i].read();
            // Unity at DC; lossy at HF. DC blocker removes subsonic accumulation.
            damped[i] += lowpass * (taps[i] - damped[i]);
            const float x = .75f * taps[i] + .25f * damped[i];
            const float hp = x - dcInput[i] + .9995f * dcOutput[i];
            dcInput[i] = x; dcOutput[i] = hp; scatter[i] = hp * loss[i];
        }
        for (size_t stride = 1; stride < 8; stride *= 2)
            for (size_t base = 0; base < 8; base += stride * 2)
                for (size_t j = 0; j < stride; ++j)
                {
                    const float a = scatter[base+j], b = scatter[base+j+stride];
                    scatter[base+j] = a+b; scatter[base+j+stride] = a-b;
                }
        for (size_t i = 0; i < 8; ++i)
            lines[i].write(scatter[i] * .35355339f + excitation * (i % 2 ? -.25f : .25f)
                          + (input[0] - input[1]) * (i % 3 ? .06f : -.06f));
        return { .5f * (taps[0]+taps[2]-taps[4]+taps[6]),
                 .5f * (taps[1]-taps[3]+taps[5]+taps[7]) };
    }
private:
    std::array<Line, 8> lines;
    std::array<Line, 4> diffusers;
    Line predelay;
    std::array<double, 8> lengths {};
    std::array<float, 8> damped {}, dcInput {}, dcOutput {}, loss {};
    double sr = 48000;
    float lowpass = 0;
    unsigned tick = 0;
};
