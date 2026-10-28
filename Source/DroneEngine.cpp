// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
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
    pitch.reset(sr, 0.025); pitch.setCurrentAndTargetValue(0);
    for (auto* macro : { &harmonyAmount, &madnessAmount })
    { macro->reset(sr, .08); macro->setCurrentAndTargetValue(0); }
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
void DroneEngine::setSacredTargets(float harmony, float madness)
{
    harmonyAmount.setTargetValue(juce::jlimit(0.0f,1.0f,harmony));
    madnessAmount.setTargetValue(juce::jlimit(0.0f,1.0f,madness));
}
void DroneEngine::Voice::stop(bool quick)
{
    held = false; releasing = true; releaseStart = envelope;
    bellReleaseStart=bellGate;
    tail = quick;
}
std::array<float,2> DroneEngine::Voice::sample(double sampleRate, float bend, float ratio, float mix, float attack, float release,
    float harmony, float madness,
    const WaveTables& tables, const std::array<std::array<float, 4>, 2>& weights)
{
    if (note < 0) return {};
    if (releasing)
        envelope = juce::jmax(0.0f, envelope - releaseStart / (static_cast<float>(sampleRate) * (tail ? 0.01f : release)));
    else
        envelope = juce::jmin(1.0f, envelope + 1.0f / (static_cast<float>(sampleRate) * attack));
    if(releasing)
        bellGate=juce::jmax(0.0f,bellGate-bellReleaseStart/static_cast<float>(sampleRate*(tail?.01:juce::jlimit(4.0f,12.0f,release+4))));
    else if(harmony>0) bellGate=juce::jmin(1.0f,bellGate+static_cast<float>(1/(sampleRate*.02)));
    if (releasing && envelope <= 0 && bellGate<=0) { note = -1; return {}; }
    std::array<float,5> modulation {};
    if (madness > 0) for(size_t i=0; i<drift.size(); ++i) modulation[i]=drift[i].next(sampleRate);
    const double drift1 = madness > 0 ? std::exp2(madness*12*modulation[0]/1200.0) : 1;
    const double drift2 = madness > 0 ? std::exp2(madness*12*modulation[1]/1200.0) : 1;
    const double dt1 = juce::jlimit(0.00000001, 0.49, frequency * bend * drift1 / sampleRate);
    const double dt2 = juce::jlimit(0.00000001, 0.49, frequency * bend * ratio * drift2 / sampleRate);
    float osc1 = 0, osc2 = 0;
    for (size_t shape = 0; shape < 4; ++shape)
    {
        if (weights[0][shape] > 0) osc1 += weights[0][shape] * tables.sample(static_cast<int>(shape), phase1, dt1);
        if (weights[1][shape] > 0) osc2 += weights[1][shape] * tables.sample(static_cast<int>(shape), phase2, dt2);
    }
    float voiceSignal = (1.0f - mix) * osc1 + mix * osc2;
    if (madness > 0)
    {
        // Band-limited waveform enrichment avoids a nonlinear waveshaper's aliasing.
        const float colour = tables.sample(2,phase1,dt1);
        const float amount = .12f*madness*(.5f+.5f*modulation[3]);
        voiceSignal += amount*(colour-voiceSignal);
    }
    float bellSignal=0;
    if (harmony > 0)
    {
        resonanceActive=true;
        if(!releasing && --bellCountdown<=0)
        {
            // Repeated strikes must rise above the already sounding drone.
            // Keep the first excitation and the sustained modal layer unchanged.
            constexpr float recurringStrikeBoost = 2.35f; // +7.4 dB
            const float strength=(.65f+.175f*(bellRandom.random()+1))
                *(bellEvents==0 ? 1.0f : recurringStrikeBoost);
            bells.strike(strength,bellRandom.random(),sampleRate);
            bellCountdown=static_cast<int>(sampleRate*(3+3*(bellRandom.random()+1)));++bellEvents;
        }
        // One +2 dB gain for the complete Harmony return. It follows both
        // resonator outputs, so strike strength, timing and decay stay intact.
        constexpr float harmonyBellGain=1.25892541179f;
        const float harmonyReturnGain=3.5f*harmony*harmonyBellGain;
        bellSignal=harmonyReturnGain*bells.process(0,frequency*bend*drift1,sampleRate,harmony)*bellGate;
        const float excitation=voiceSignal + .35f*tables.sample(2,phase1,dt1);
        voiceSignal += harmonyReturnGain*resonance.process(excitation,frequency*bend*drift1,sampleRate,harmony);
    }
    else if(resonanceActive) { resonance.reset(); bells.reset(false);bellCountdown=0;bellGate=0;resonanceActive=false; }
    const float out = (voiceSignal * envelope + bellSignal) * velocity * (1 + .15f*madness*modulation[2]);
    phase1 += dt1; phase2 += dt2;
    if (phase1 >= 1) phase1 -= 1;
    if (phase2 >= 1) phase2 -= 1;
    if (madness == 0) return {out,out};
    const float pan=.30f*madness*modulation[4];
    return {out*std::sqrt(1-pan),out*std::sqrt(1+pan)};
}
void DroneEngine::noteOn(int channel, int note, float velocity, int source)
{
    if(note<0 || note>127 || (reservedOctaves & (1u<<(note/12))))return;
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
    chosen->resonance.reset();
    chosen->bells.reset(false);chosen->bellRandom.seed=static_cast<uint32_t>(clock*277803737u+note*747796405u)|1u;
    for(size_t i=0; i<chosen->drift.size(); ++i)
        chosen->drift[i].reset(static_cast<uint32_t>(clock*747796405u + static_cast<uint64_t>(note)*2891336453u + i*277803737u),sr);
}
void DroneEngine::setReservedOctaves(uint16_t mask)
{
    if(mask==reservedOctaves)return;
    reservedOctaves=mask;
    for(auto& v:voices)if(v.note>=0 && (mask & (1u<<(v.note/12))))v.stop(true);
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
    const auto stereo=nextStereoSample(); return .5f*(stereo[0]+stereo[1]);
}
std::array<float,2> DroneEngine::nextStereoSample()
{
    for (size_t osc = 0; osc < 2; ++osc)
        if (waveRemaining[osc] > 0)
        {
            for (size_t shape = 0; shape < 4; ++shape)
                waveWeights[osc][shape] += ((static_cast<int>(shape) == waveTargets[osc] ? 1.0f : 0.0f) - waveWeights[osc][shape])
                                        / static_cast<float>(waveRemaining[osc]);
            --waveRemaining[osc];
        }
    const auto bend = std::exp2(pitch.getNextValue() / 1200.0f);
    const auto ratio = std::exp2(controls[0].getNextValue() / 1200.0f);
    const auto mix = controls[1].getNextValue();
    const auto attack = juce::jmax(0.005f, controls[2].getNextValue());
    const auto release = juce::jmax(0.005f, controls[3].getNextValue());
    const auto gain = controls[4].getNextValue();
    const float harmony=harmonyAmount.getNextValue();
    // Fixed depth equals 0.9.0 with Hesitation settled at 127. E8 is smoothed once.
    const float madness=2.0f*madnessAmount.getNextValue();
    std::array<float,2> sum {};
    auto render = [&](auto& bank) { for(auto& v:bank)
    {
        const auto sample=v.sample(sr,bend,ratio,mix,attack,release,harmony,madness,*waveTables,waveWeights);
        sum[0]+=sample[0]; sum[1]+=sample[1];
    } };
    render(voices); render(tails);
    // Fixed per-voice gain: changing the polyphony limit never ducks existing notes.
    // The reference divisor remains eight, independent of the maximum voice count.
    for(auto& channel:sum)
    {
        const float signal=channel*(0.8f/(2.0f*voiceCount))*outputBoost;
        channel=externalOutput ? signal : protectOutput(signal*gain);
    }
    return sum;
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
std::array<float,128> DroneEngine::envelopeLevels() const
{
    std::array<float,128> levels {};
    auto collect=[&](const auto& bank){for(const auto& v:bank)if(v.note>=0)
        levels[static_cast<size_t>(v.note)]=juce::jmax(levels[static_cast<size_t>(v.note)],v.envelope);};
    collect(voices);collect(tails);return levels;
}
uint64_t DroneEngine::harmonyEvents() const
{
    uint64_t total=0;for(const auto& v:voices)total+=v.bellEvents;for(const auto& v:tails)total+=v.bellEvents;return total;
}
}
