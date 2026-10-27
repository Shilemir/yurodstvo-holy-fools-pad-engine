// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

class ThreeBandEQ
{
public:
    void prepare(double sampleRate, std::array<float, 3> initial = {});
    void reset() noexcept { states = {}; }
    void setTargets(const std::array<float, 3>& db);
    std::array<float, 2> process(std::array<float, 2> input);
    static float ccToDb(int value)
    {
        value = juce::jlimit(0, 127, value);
        return value <= 64 ? 12.0f * (value - 64) / 64.0f : 12.0f * (value - 64) / 63.0f;
    }
private:
    using Coefficients = std::array<double, 5>;
    struct State { double z1 = 0, z2 = 0; };
    std::array<Coefficients, 3> makeCoefficients(const std::array<float, 3>& db) const;
    std::array<Coefficients, 3> coefficients {};
    std::array<std::array<State, 3>, 2> states {};
    std::array<juce::SmoothedValue<float>, 3> gains;
    std::array<float, 3> targets {};
    double sr = 48000;
    uint32_t clock = 0;
};
