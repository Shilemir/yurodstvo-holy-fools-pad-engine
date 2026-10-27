#include "DroneEngine.h"
#include "OutputStage.h"
#include <cmath>
#include <memory>

namespace yuro
{
void DroneEngine::prepare(double sampleRate, bool externalOutputStage)
{
    externalOutput = externalOutputStage;
    static const auto sharedTables = std::make_unique<const WaveTables>();
    waveTables = sharedTables.get();
    waveTargets = { 0, 0 }; waveRemaining = { 0, 0 };
    waveWeights = { { { 1, 0, 0, 0 }, { 1, 0, 0, 0 } } };
    sr = sampleRate > 0 ? sampleRate : 48000;
    voices = {}; tails = {}; pedals = {}; clock = 0; nextTail = 0; polyphony = voiceCount;
    constexpr std::array<float, parameterCount> defaults { 7, 0.5f, 4, 8, 0.65f };
    for (size_t i = 0; i < controls.size(); ++i)
    {
        controls[i].reset(sr, i == 0 ? 0.08 : 0.04);
        controls[i].setCurrentAndTargetValue(defaults[i]);
    }
}
void DroneEngine::setWaveforms(const std::array<int, 2>& shapes)
{
    for (size_t osc = 0; osc < shapes.size(); ++osc)
    {
        const int target = juce::jlimit(0, 3, shapes[osc]);
        if (target != waveTargets[osc])
        {
            waveTargets[osc] = target;
            waveRemaining[osc] = juce::jmax(1, juce::roundToInt(sr * 0.025));
        }
    }
}
void DroneEngine::setTargets(const std::array<float, parameterCount>& values)
{
    for (size_t i = 0; i < controls.size(); ++i)
        controls[i].setTargetValue(values[i]);
}
void DroneEngine::setPolyphony(int limit)
{
    limit = juce::jlimit(1, maxVoiceCount, limit);
    if (limit == polyphony) return;
    polyphony = limit;
    // Keep lower voice slots; retire excess slots over 10 ms without allocation.
    for (size_t i = static_cast<size_t>(limit); i < voices.size(); ++i)
        if (voices[i].note >= 0 && !voices[i].tail) voices[i].stop(true);
}
void DroneEngine::Voice::stop(bool quick)
{
    held = false; releasing = true; releaseStart = envelope;
    tail = quick;
}
float DroneEngine::Voice::sample(double sampleRate, float ratio, float mix, float attack, float release,
    const WaveTables& tables, const std::array<std::array<float, 4>, 2>& weights)
{
    if (note < 0) return 0;
    if (releasing)
        envelope = juce::jmax(0.0f, envelope - releaseStart / (static_cast<float>(sampleRate) * (tail ? 0.01f : release)));
    else
        envelope = juce::jmin(1.0f, envelope + 1.0f / (static_cast<float>(sampleRate) * attack));
    if (releasing && envelope <= 0) { note = -1; return 0; }
    const double dt1 = juce::jlimit(0.00000001, 0.49, frequency / sampleRate);
    const double dt2 = juce::jlimit(0.00000001, 0.49, frequency * ratio / sampleRate);
    float osc1 = 0, osc2 = 0;
    for (size_t shape = 0; shape < 4; ++shape)
    {
        if (weights[0][shape] > 0) osc1 += weights[0][shape] * tables.sample(static_cast<int>(shape), phase1, dt1);
        if (weights[1][shape] > 0) osc2 += weights[1][shape] * tables.sample(static_cast<int>(shape), phase2, dt2);
    }
    const float out = ((1.0f - mix) * osc1 + mix * osc2) * envelope * velocity;
    phase1 += dt1; phase2 += dt2;
    if (phase1 >= 1) phase1 -= 1;
    if (phase2 >= 1) phase2 -= 1;
    return out;
}
void DroneEngine::noteOn(int channel, int note, float velocity, int source)
{
    if (velocity <= 0) { noteOff(channel, note, source); return; }
    Voice* chosen = nullptr;
    for (int i = 0; i < polyphony; ++i) if (voices[static_cast<size_t>(i)].note < 0) { chosen = &voices[static_cast<size_t>(i)]; break; }
    if (chosen == nullptr)
    {
        // Preserve existing fading tails under an extreme burst of >8 steals/10ms.
        // Dropping an excess Note On is preferable to cutting a nonzero waveform.
        size_t freeTail = tails.size();
        for (size_t i = 0; i < tails.size(); ++i)
        {
            const auto index = (nextTail + i) % tails.size();
            if (tails[index].note < 0) { freeTail = index; break; }
        }
        if (freeTail == tails.size()) return;
        chosen = &voices[0];
        for (int i = 0; i < polyphony; ++i)
        {
            auto& v = voices[static_cast<size_t>(i)];
            if ((v.releasing && !chosen->releasing) || (v.releasing == chosen->releasing && v.age < chosen->age)) chosen = &v;
        }
        tails[freeTail] = *chosen;
        tails[freeTail].stop(true);
        nextTail = (freeTail + 1) % tails.size();
    }
    *chosen = {};
    chosen->note = note; chosen->channel = channel; chosen->source = source; chosen->held = true;
    chosen->velocity = juce::jlimit(0.0f, 1.0f, velocity);
    chosen->frequency = juce::MidiMessage::getMidiNoteInHertz(note);
    chosen->age = ++clock;
}
void DroneEngine::noteOff(int channel, int note, int source)
{
    for (auto& v : voices)
        if (v.note == note && v.channel == channel && v.source == source && v.held)
        {
            v.held = false;
            if (source == 1 || !pedals[static_cast<size_t>(channel - 1)]) v.stop();
        }
}
void DroneEngine::sustain(int channel, bool down)
{
    pedals[static_cast<size_t>(channel - 1)] = down;
    if (!down) for (auto& v : voices) if (v.channel == channel && !v.held && !v.releasing) v.stop();
}
void DroneEngine::allNotesOff(int channel, bool fast)
{
    if (channel > 0) pedals[static_cast<size_t>(channel - 1)] = false;
    else pedals.fill(false);
    auto stop = [&](auto& bank) { for (auto& v : bank)
        if (v.note >= 0 && (channel == 0 || v.channel == channel)) v.stop(fast); };
    stop(voices); stop(tails);
}
float DroneEngine::nextSample()
{
    for (size_t osc = 0; osc < 2; ++osc)
        if (waveRemaining[osc] > 0)
        {
            for (size_t shape = 0; shape < 4; ++shape)
                waveWeights[osc][shape] += ((static_cast<int>(shape) == waveTargets[osc] ? 1.0f : 0.0f) - waveWeights[osc][shape])
                                        / static_cast<float>(waveRemaining[osc]);
            --waveRemaining[osc];
        }
    const auto ratio = std::exp2(controls[0].getNextValue() / 1200.0f);
    const auto mix = controls[1].getNextValue();
    const auto attack = juce::jmax(0.005f, controls[2].getNextValue());
    const auto release = juce::jmax(0.005f, controls[3].getNextValue());
    const auto gain = controls[4].getNextValue();
    float sum = 0;
    for (auto& v : voices) sum += v.sample(sr, ratio, mix, attack, release, *waveTables, waveWeights);
    for (auto& v : tails) sum += v.sample(sr, ratio, mix, attack, release, *waveTables, waveWeights);
    // Fixed per-voice gain: changing the polyphony limit never ducks existing notes.
    // The reference divisor remains eight, independent of the maximum voice count.
    const float signal = sum * (0.8f / (2.0f * voiceCount)) * outputBoost;
    return externalOutput ? signal : protectOutput(signal * gain);
}
int DroneEngine::activeVoices() const
{
    int count = 0;
    for (const auto& v : voices) if (v.note >= 0) ++count;
    return count;
}
bool DroneEngine::sounding(int note) const
{
    for (const auto& v : voices) if (v.note == note) return true;
    return false;
}
}
