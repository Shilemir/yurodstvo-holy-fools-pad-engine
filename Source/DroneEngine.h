// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include "Waveforms.h"
#include "SacredVoice.h"

namespace yuro
{
constexpr int voiceCount = 8;
constexpr int maxVoiceCount = 16;
constexpr int parameterCount = 5;
inline constexpr std::array<const char*, parameterCount> parameterIds { "detune", "mix", "attack", "release", "master" };
inline constexpr std::array<int, parameterCount> midiCC { 74, 71, 76, 77, 82 };

// No allocation, locks or host callbacks in the synthesis engine.
class DroneEngine
{
public:
    void prepare(double sampleRate, bool externalOutputStage = false);
    void setTargets(const std::array<float, parameterCount>& values);
    void setPolyphony(int limit);
    void setPitchCents(float cents) { pitch.setTargetValue(juce::jlimit(-100.0f, 100.0f, cents)); }
    void setWaveforms(const std::array<int, 2>& shapes);
    void noteOn(int channel, int note, float velocity, int source = 0);
    void noteOff(int channel, int note, int source = 0);
    void sustain(int channel, bool down);
    void allNotesOff(int channel, bool fast);
    float nextSample();
    std::array<float,2> nextStereoSample();
    void setSacredTargets(float harmony, float madness);
    int activeVoices() const;
    bool sounding(int note) const;
    std::array<float,128> envelopeLevels() const;
    uint64_t harmonyEvents() const;
private:
    struct Voice
    {
        int note = -1, channel = 1, source = 0; // 0 = host MIDI, 1 = on-screen mouse.
        uint64_t age = 0;
        double phase1 = 0, phase2 = 0, frequency = 0;
        float envelope = 0, velocity = 0, releaseStart = 0;
        bool releasing = false, held = false, tail = false;
        SacredResonance resonance;
        bool resonanceActive = false;
        SacredResonance bells;
        OrganicDrift bellRandom;
        int bellCountdown=0;
        float bellGate=0,bellReleaseStart=0;
        uint64_t bellEvents=0;
        std::array<OrganicDrift,5> drift;
        std::array<float,2> sample(double sr, float bend, float ratio, float mix, float attack, float release,
                     float harmony, float madness,
                     const WaveTables&, const std::array<std::array<float, 4>, 2>& weights);
        void stop(bool quick = false);
    };
    std::array<Voice, maxVoiceCount> voices {};
    std::array<Voice, maxVoiceCount * 2> tails {};
    int polyphony = voiceCount;
    std::array<bool, 16> pedals {};
    std::array<juce::SmoothedValue<float>, parameterCount> controls;
    juce::SmoothedValue<float> pitch;
    juce::SmoothedValue<float> harmonyAmount, madnessAmount;
    double sr = 48000;
    uint64_t clock = 0;
    size_t nextTail = 0;
    const WaveTables* waveTables = nullptr;
    bool externalOutput = false;
    std::array<std::array<float, 4>, 2> waveWeights { { { 1, 0, 0, 0 }, { 1, 0, 0, 0 } } };
    std::array<int, 2> waveTargets { 0, 0 }, waveRemaining { 0, 0 };
};
}
