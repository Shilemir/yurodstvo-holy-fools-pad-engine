// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#include "Shimmer.h"

void OctavePitchShifter::prepare(double sr)
{
    grain = { sr * 0.1, sr * 0.113 }; minimumDelay = sr * 0.02;
    for (auto& channel : ring) channel.assign(static_cast<size_t>(std::ceil(grain[1] + minimumDelay)) + 8, 0);
    for (size_t i = 0; i < windows.size(); ++i)
        windows[i] = static_cast<float>(0.5 - 0.5 * std::cos(juce::MathConstants<double>::twoPi * i / (windows.size() - 1)));
    clear();
}
void OctavePitchShifter::clear()
{
    for (auto& channel : ring) std::fill(channel.begin(), channel.end(), 0);
    write = 0; phase = {};
}
float OctavePitchShifter::window(double p) const
{
    const double index = p * (windows.size() - 1); const auto i = static_cast<size_t>(index);
    return windows[i] + static_cast<float>(index - i) * (windows[i + 1] - windows[i]);
}
float OctavePitchShifter::read(size_t ch, double delay) const
{
    const int length = static_cast<int>(ring[ch].size());
    double index = static_cast<double>(write) - delay;
    if (index < 0) index += length;
    const int base = static_cast<int>(index); const float f = static_cast<float>(index - base);
    auto sample = [&](int offset) { return ring[ch][static_cast<size_t>((base + offset + length) % length)]; };
    const float a = sample(-1), b = sample(0), c = sample(1), d = sample(2);
    return b + 0.5f * f * (c - a + f * (2*a - 5*b + 4*c - d + f * (3*(b-c) + d-a)));
}
std::array<float, 2> OctavePitchShifter::process(std::array<float, 2> input)
{
    std::array<float, 2> output {};
    for (size_t ch = 0; ch < 2; ++ch)
    {
        ring[ch][write] = input[ch];
        for (int head = 0; head < 2; ++head)
        {
            double p = phase[ch] + 0.5 * head;
            p -= std::floor(p);
            // Decreasing delay by one sample per input sample yields +12 semitones.
            output[ch] += window(p) * read(ch, minimumDelay + (1-p) * grain[ch]);
        }
        phase[ch] += 1 / grain[ch]; if (phase[ch] >= 1) phase[ch] -= 1;
    }
    write = (write + 1) % ring[0].size();
    return output;
}
void Shimmer::prepare(double sr)
{
    pitch.prepare(sr); space.setSampleRate(sr);
    juce::Reverb::Parameters p;
    p.roomSize = 0.87f; p.damping = 0.53f; p.width = 1;
    p.dryLevel = 0; p.wetLevel = 1.0f / 3; p.freezeMode = 0; space.setParameters(p);
    constexpr std::array<double, 4> q { 0.50979558, 0.60134489, 0.89997622, 2.56291545 };
    for (size_t ch = 0; ch < 2; ++ch)
    {
        for (size_t band = 0; band < q.size(); ++band)
            antiAlias[ch][band].setCoefficients(juce::IIRCoefficients::makeLowPass(sr, juce::jmin(6000.0, sr * 0.12), q[band]));
        highPass[ch].setCoefficients(juce::IIRCoefficients::makeHighPass(sr, 90));
        damping[ch].setCoefficients(juce::IIRCoefficients::makeLowPass(sr, juce::jmin(6500.0, sr * 0.3)));
    }
    target = 0; amount.reset(sr, 0.08); clear();
}
void Shimmer::clear()
{
    pitch.clear(); space.reset(); feedback = {};
    for (size_t ch = 0; ch < 2; ++ch)
    {
        for (auto& filter : antiAlias[ch]) filter.reset();
        highPass[ch].reset(); damping[ch].reset();
    }
    amount.setCurrentAndTargetValue(target); active = target > 0;
}
void Shimmer::setAmount(float value)
{
    target = juce::jlimit(0.0f, 1.0f, value); amount.setTargetValue(target);
    if (target > 0) active = true;
}
std::array<float, 2> Shimmer::process(std::array<float, 2> input)
{
    if (!active) return {};
    const float level = amount.getNextValue();
    if (level == 0 && !amount.isSmoothing()) { clear(); return {}; }
    for (size_t ch = 0; ch < 2; ++ch)
    {
        // Bounded send/feedback even with extreme boosted EQ; no dry-path AGC.
        input[ch] = 0.8f * std::tanh((0.32f * input[ch] + (0.08f + 0.27f * level) * feedback[ch]) / 0.8f);
        for (auto& filter : antiAlias[ch]) input[ch] = filter.processSingleSampleRaw(input[ch]);
    }
    auto wet = pitch.process(input);
    space.processStereo(&wet[0], &wet[1], 1);
    for (size_t ch = 0; ch < 2; ++ch)
    {
        const float filtered = damping[ch].processSingleSampleRaw(highPass[ch].processSingleSampleRaw(wet[ch]));
        feedback[ch] = std::isfinite(filtered) ? 0.9f * std::tanh(filtered / 0.9f) : 0;
        wet[ch] = level * 1.8f * feedback[ch];
    }
    return wet;
}
