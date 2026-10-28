// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <juce_core/juce_core.h>
#include <array>

namespace yuro
{
inline constexpr std::array<const char*,13> soundIds {
    "shimmer", "mix", "attack", "release", "reverb", "delay", "harmony", "madness",
    "eqBass", "eqMid", "eqTreble", "osc1Waveform", "osc2Waveform" };
struct SoundPreset { std::array<float,13> values {}; };
inline constexpr std::array<const char*,4> archetypeNames {
    "THE HERMIT", "THE MARTYR", "THE INQUISITOR", "THE HIEROPHANT" };
// Plain parameter values, not host-normalized values. Dualism stores its legacy 0..1 mix.
inline constexpr std::array<SoundPreset,4> archetypes {{
    {{ .437f,.425f,5.0f,10.0f,.65f,.15f,.10f,.08f,-2,-1,-2,0,1 }},
    {{ .10f,.60f,.15f,5.0f,.45f,.35f,.20f,.93f,6,2,-7.1f,2,3 }},
    {{ .08f,.20f,.005f,2.0f,.20f,.20f,1.0f,.666f,3.4f,3,5,2,3 }},
    {{ .80f,.50f,3.0f,15.0f,.90f,.35f,1.0f,.15f,-1.0f,2,3,1,0 }}
}};

class CustomPresetStore
{
public:
    explicit CustomPresetStore(juce::File directory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory).getChildFile("YURODSTVO"))
        : file(directory.getChildFile("custom-preset-v1.json")) {}
    bool save(const SoundPreset& sound) const;
    bool load(SoundPreset& sound) const;
    const juce::File& location() const { return file; }
private:
    juce::File file;
};
}
