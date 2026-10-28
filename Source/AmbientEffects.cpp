// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#include "AmbientEffects.h"

void AmbientEffects::prepare(double sampleRate, float initialMaster)
{
    sr = sampleRate > 0 ? sampleRate : 48000;
    shimmer.prepare(sr);
    for (auto& line : delayLines) line.assign(static_cast<size_t>(std::ceil(sr)) + 1, 0);
    delaySamples = { juce::roundToInt(sr * 0.57), juce::roundToInt(sr * 0.76) };
    damping = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * 2800.0f / static_cast<float>(sr));
    reverb.prepare(sr);
    roomMix.reset(sr, 0.06); echoMix.reset(sr, 0.06); masterGain.reset(sr, 0.04); panicGain.reset(sr, 0.01);
    roomMix.setCurrentAndTargetValue(0); echoMix.setCurrentAndTargetValue(0);
    masterGain.setCurrentAndTargetValue(initialMaster); panicGain.setCurrentAndTargetValue(1);
    clear();
}
void AmbientEffects::clear()
{
    for (auto& line : delayLines) std::fill(line.begin(), line.end(), 0.0f);
    filtered = {}; position = 0; reverb.clear(); shimmer.clear(); clearing = false;
}
void AmbientEffects::beginPanic() { panicGain.setTargetValue(0); clearing = true; }
void AmbientEffects::setTargets(float room, float echo, float master)
{
    roomMix.setTargetValue(juce::jlimit(0.0f, 1.0f, room));
    echoMix.setTargetValue(juce::jlimit(0.0f, 1.0f, echo));
    masterGain.setTargetValue(juce::jlimit(0.0f, 1.0f, master));
}
std::array<float, 2> AmbientEffects::process(float mono)
{
    return process(std::array<float, 2> { mono, mono });
}
std::array<float, 2> AmbientEffects::process(std::array<float, 2> inputStereo)
{
    const float r = roomMix.getNextValue(), d = echoMix.getNextValue();
    const int length = static_cast<int>(delayLines[0].size());
    std::array<float, 2> taps {};
    for (size_t ch = 0; ch < 2; ++ch)
    {
        const int read = (position + length - delaySamples[ch]) % length;
        taps[ch] = delayLines[ch][static_cast<size_t>(read)];
        filtered[ch] += damping * (taps[ch] - filtered[ch]);
    }
    const float feedback = 0.22f + 0.66f * d;
    for (size_t ch = 0; ch < 2; ++ch)
    {
        const float input = inputStereo[ch] + feedback * (0.35f * filtered[ch] + 0.65f * filtered[1 - ch]);
        // Linear at normal levels; soft protection only for exceptional loop peaks.
        delayLines[ch][static_cast<size_t>(position)] = yuro::protectOutput(input);
    }
    position = (position + 1) % length;
    // Parallel sends: raising one effect never turns down the other returns.
    const auto wet = reverb.process(inputStereo, r);
    const auto octave = shimmer.process(inputStereo);
    const float gain = masterGain.getNextValue() * panicGain.getNextValue();
    std::array<float, 2> result {};
    for (size_t ch = 0; ch < 2; ++ch)
        result[ch] = yuro::protectOutput(((1 - 0.50f * r) * inputStereo[ch]
                      + 2.0f * r * wet[ch] + 1.15f * d * taps[ch] + octave[ch]) * gain);
    if (clearing && !panicGain.isSmoothing()) { clear(); panicGain.setTargetValue(1); }
    return result;
}
