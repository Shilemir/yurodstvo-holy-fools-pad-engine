// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#include "AmbientEffects.h"
#include "ParameterLimits.h"

void AmbientEffects::prepare(double sampleRate, float initialMaster)
{
    sr = sampleRate > 0 ? sampleRate : 48000;
    shimmer.prepare(sr);
    // A dotted quarter note at the slowest supported 30 BPM lasts 3 seconds.
    for (auto& line : delayLines) line.assign(static_cast<size_t>(std::ceil(sr*3.05)) + 2, 0);
    delaySamples = { juce::roundToInt(sr * .5), juce::roundToInt(sr * .75) };
    requestedDelaySamples=nextDelaySamples=delaySamples;
    delayFadeRemaining=0;delayFadeLength=juce::jmax(1,juce::roundToInt(sr*.06));
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
void AmbientEffects::setTempoBpm(double bpm) noexcept
{
    // Invalid host data leaves the last valid tempo in place (120 BPM on startup).
    if(!std::isfinite(bpm) || bpm<30.0 || bpm>300.0)return;
    const int length=static_cast<int>(delayLines[0].size());
    const double quarterSamples=sr*60.0/bpm;
    requestedDelaySamples={juce::jlimit(1,length-1,juce::roundToInt(quarterSamples)),
                           juce::jlimit(1,length-1,juce::roundToInt(quarterSamples*1.5))};
}
void AmbientEffects::setTargets(float room, float echo, float master)
{
    roomMix.setTargetValue(juce::jlimit(0.0f, 1.0f, room));
    echoMix.setTargetValue(juce::jlimit(0.0f, 1.0f, echo));
    masterGain.setTargetValue(juce::jlimit(0.0f, yuro::maxMasterGain, master));
}
std::array<float, 2> AmbientEffects::process(float mono)
{
    return process(std::array<float, 2> { mono, mono });
}
std::array<float, 2> AmbientEffects::process(std::array<float, 2> inputStereo)
{
    const float r = roomMix.getNextValue(), d = echoMix.getNextValue();
    const int length = static_cast<int>(delayLines[0].size());
    if(delayFadeRemaining==0 && requestedDelaySamples!=delaySamples)
    {nextDelaySamples=requestedDelaySamples;delayFadeRemaining=delayFadeLength;}
    const float delayFade=delayFadeRemaining>0
        ? static_cast<float>(delayFadeLength-delayFadeRemaining)/delayFadeLength : 0.0f;
    std::array<float, 2> taps {};
    for (size_t ch = 0; ch < 2; ++ch)
    {
        const int read = (position + length - delaySamples[ch]) % length;
        taps[ch] = delayLines[ch][static_cast<size_t>(read)];
        if(delayFadeRemaining>0)
        {
            const int nextRead=(position+length-nextDelaySamples[ch])%length;
            taps[ch]=taps[ch]*(1-delayFade)+delayLines[ch][static_cast<size_t>(nextRead)]*delayFade;
        }
        filtered[ch] += damping * (taps[ch] - filtered[ch]);
    }
    if(delayFadeRemaining>0 && --delayFadeRemaining==0)delaySamples=nextDelaySamples;
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
