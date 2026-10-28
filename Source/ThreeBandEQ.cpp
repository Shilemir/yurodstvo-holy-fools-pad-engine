// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#include "ThreeBandEQ.h"

std::array<ThreeBandEQ::Coefficients, 3> ThreeBandEQ::makeCoefficients(const std::array<float, 3>& db) const
{
    const std::array<juce::IIRCoefficients, 3> c {
        juce::IIRCoefficients::makeLowShelf(sr, juce::jmin(120.0, sr * 0.1), 0.70710678, juce::Decibels::decibelsToGain(db[0])),
        juce::IIRCoefficients::makePeakFilter(sr, juce::jmin(1000.0, sr * 0.2), 0.8, juce::Decibels::decibelsToGain(db[1])),
        juce::IIRCoefficients::makeHighShelf(sr, juce::jmin(6000.0, sr * 0.4), 0.70710678, juce::Decibels::decibelsToGain(db[2])) };
    std::array<Coefficients, 3> result {};
    for (size_t band = 0; band < 3; ++band) for (size_t i = 0; i < 5; ++i) result[band][i] = c[band].coefficients[i];
    return result;
}
void ThreeBandEQ::prepare(double sampleRate, std::array<float, 3> initial)
{
    sr = sampleRate > 0 ? sampleRate : 48000;
    states = {}; clock = 0;
    for (size_t i = 0; i < 3; ++i)
    {
        targets[i] = juce::jlimit(-yuro::maxEqDb, yuro::maxEqDb, initial[i]);
        gains[i].reset(sr, 0.05); gains[i].setCurrentAndTargetValue(targets[i]);
    }
    coefficients = makeCoefficients(targets);
}
void ThreeBandEQ::setTargets(const std::array<float, 3>& db)
{
    for (size_t i = 0; i < 3; ++i)
    {
        const float value = juce::jlimit(-yuro::maxEqDb, yuro::maxEqDb, db[i]);
        targets[i] = value; gains[i].setTargetValue(value);
    }
}
std::array<float, 2> ThreeBandEQ::process(std::array<float, 2> input)
{
    const std::array<float, 3> db { gains[0].getNextValue(), gains[1].getNextValue(), gains[2].getNextValue() };
    if (clock++ % 16 == 0) coefficients = makeCoefficients(db);
    for (size_t ch = 0; ch < 2; ++ch)
    {
        double x = input[ch];
        for (size_t band = 0; band < 3; ++band)
        {
            const auto& b = coefficients[band]; auto& s = states[ch][band];
            const double y = b[0] * x + s.z1;
            s.z1 = b[1] * x - b[3] * y + s.z2; s.z2 = b[2] * x - b[4] * y;
            if (std::abs(s.z1) < 1.0e-28) s.z1 = 0;
            if (std::abs(s.z2) < 1.0e-28) s.z2 = 0;
            if (!std::isfinite(y) || std::abs(y) > 1.0e6) { s = {}; x = 0; } else x = y;
        }
        input[ch] = static_cast<float>(x);
    }
    return input;
}
