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
    {{ .184f,.3162f,4.0f,8.0f,.541f,.15f,.33f,.08f,0,0,0,0,1 }},
    {{ .054f,.3605f,1.011f,13.905f,.237f,.498f,1.0f,1.0f,-.6f,.3f,6.6f,1,0 }},
    {{ 1.0f,.2045f,.005f,1.115f,.35f,0,1.0f,.666f,-7.5f,0,2.9f,3,0 }},
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
