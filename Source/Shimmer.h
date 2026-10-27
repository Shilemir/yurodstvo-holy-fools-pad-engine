#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <vector>

// Four Hann-windowed read heads (two per channel), cubic interpolation, 2x speed.
// All storage is allocated in prepare; no locks, allocation or FFT plans in audio.
class OctavePitchShifter
{
public:
    void prepare(double sampleRate);
    void clear();
    std::array<float, 2> process(std::array<float, 2> input);
private:
    float read(size_t channel, double delay) const;
    float window(double phase) const;
    std::array<std::vector<float>, 2> ring;
    std::array<float, 2049> windows {};
    size_t write = 0;
    std::array<double, 2> phase {}, grain { 4800, 5424 };
    double minimumDelay = 960;
};

class Shimmer
{
public:
    void prepare(double sampleRate);
    void clear();
    void setAmount(float value);
    std::array<float, 2> process(std::array<float, 2> input);
private:
    OctavePitchShifter pitch;
    juce::Reverb space; // Independent from E5's reverb.
    std::array<std::array<juce::IIRFilter, 4>, 2> antiAlias;
    std::array<juce::IIRFilter, 2> highPass, damping;
    std::array<float, 2> feedback {};
    juce::SmoothedValue<float> amount;
    float target = 0;
    bool active = false;
};
