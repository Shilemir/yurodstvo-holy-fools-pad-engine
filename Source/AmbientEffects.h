// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include "OutputStage.h"
#include "Shimmer.h"
#include "CathedralReverb.h"

class AmbientEffects
{
public:
    void prepare(double sampleRate, float initialMaster);
    void setTargets(float reverbAmount, float delayAmount, float master);
    void setTempoBpm(double bpm) noexcept;
    void setShimmer(float value) { shimmer.setAmount(value); }
    std::array<float, 2> process(float mono);
    std::array<float, 2> process(std::array<float, 2> stereo);
    void beginPanic();
    bool isPanicFading() const noexcept { return clearing; }
private:
    void clear();
    CathedralReverb reverb;
    Shimmer shimmer;
    std::array<std::vector<float>, 2> delayLines;
    std::array<int, 2> delaySamples {};
    std::array<int, 2> requestedDelaySamples {}, nextDelaySamples {};
    int delayFadeRemaining=0, delayFadeLength=1;
    std::array<float, 2> filtered {};
    juce::SmoothedValue<float> roomMix, echoMix, masterGain, panicGain;
    int position = 0;
    double sr = 48000;
    float damping = 0;
    bool clearing = false;
};
