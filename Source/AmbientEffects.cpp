#include "AmbientEffects.h"

void AmbientEffects::prepare(double sampleRate, float initialMaster)
{
    sr = sampleRate > 0 ? sampleRate : 48000;
    shimmer.prepare(sr);
    for (auto& line : delayLines) line.assign(static_cast<size_t>(std::ceil(sr)) + 1, 0);
    delaySamples = { juce::roundToInt(sr * 0.57), juce::roundToInt(sr * 0.76) };
    damping = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * 2800.0f / static_cast<float>(sr));
    juce::Reverb::Parameters p;
    p.roomSize = 0.58f; p.damping = 0.55f; p.width = 1;
    p.dryLevel = 0; p.wetLevel = 1.0f / 3.0f; p.freezeMode = 0;
    reverb.setParameters(p); reverb.setSampleRate(sr);
    roomMix.reset(sr, 0.06); echoMix.reset(sr, 0.06); masterGain.reset(sr, 0.04); panicGain.reset(sr, 0.01);
    roomMix.setCurrentAndTargetValue(0); echoMix.setCurrentAndTargetValue(0);
    masterGain.setCurrentAndTargetValue(initialMaster); panicGain.setCurrentAndTargetValue(1);
    clear();
}
void AmbientEffects::clear()
{
    for (auto& line : delayLines) std::fill(line.begin(), line.end(), 0.0f);
    filtered = {}; position = 0; reverb.reset(); shimmer.clear(); clearing = false;
}
void AmbientEffects::beginPanic() { panicGain.setTargetValue(0); clearing = true; }
void AmbientEffects::setTargets(float room, float echo, float master)
{
    roomMix.setTargetValue(juce::jlimit(0.0f, 1.0f, room));
    echoMix.setTargetValue(juce::jlimit(0.0f, 1.0f, echo));
    masterGain.setTargetValue(juce::jlimit(0.0f, 1.0f, master));
    auto p = reverb.getParameters();
    p.roomSize = 0.58f + 0.29f * juce::jlimit(0.0f, 1.0f, room);
    reverb.setParameters(p); // Same audio thread; JUCE smooths internal feedback.
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
    const float feedback = 0.15f + 0.50f * d;
    for (size_t ch = 0; ch < 2; ++ch)
    {
        const float input = inputStereo[ch] + feedback * (0.8f * filtered[ch] + 0.2f * filtered[1 - ch]);
        delayLines[ch][static_cast<size_t>(position)] = 0.95f * std::tanh(input / 0.95f);
    }
    position = (position + 1) % length;
    std::array<float, 2> dry { inputStereo[0] * (1 - 0.18f * d) + 0.40f * d * taps[0],
                               inputStereo[1] * (1 - 0.18f * d) + 0.40f * d * taps[1] };
    auto wet = dry;
    reverb.processStereo(&wet[0], &wet[1], 1);
    const auto octave = shimmer.process(inputStereo);
    const float gain = masterGain.getNextValue() * panicGain.getNextValue();
    std::array<float, 2> result {};
    for (size_t ch = 0; ch < 2; ++ch)
        result[ch] = yuro::protectOutput(((1 - 0.22f * r) * dry[ch] + 0.42f * r * wet[ch] + octave[ch]) * gain);
    if (clearing && !panicGain.isSmoothing()) { clear(); panicGain.setTargetValue(1); }
    return result;
}
