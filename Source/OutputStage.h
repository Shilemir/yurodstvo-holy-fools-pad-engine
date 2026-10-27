// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include <cmath>

namespace yuro
{
// +6.5 dB over 0.1 (+3 dB over 0.6), with fixed per-voice gain. The memoryless
// C1 knee catches actual large peaks from dense chords, boosted EQ or effects.
// Below the knee it is exactly linear: no AGC, allocation or added latency.
inline constexpr float outputBoost = 1.496235656f * 1.412537545f; // +3 dB over 0.6, +6.5 dB over 0.1.
inline constexpr float outputKnee = 0.75f;
inline constexpr float outputCeiling = 0.94f;
inline float protectOutput(float sample) noexcept
{
    const float magnitude = std::abs(sample);
    if (magnitude <= outputKnee) return sample;
    constexpr float span = outputCeiling - outputKnee;
    return std::copysign(outputKnee + span * std::tanh((magnitude - outputKnee) / span), sample);
}
}
