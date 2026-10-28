// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DroneEngine.h"
#include "ParameterLimits.h"
#include "AmbientEffects.h"
#include "PadRouter.h"
#include "ThreeBandEQ.h"
#include "MouseNoteInput.h"
#include "SoundPresets.h"

class DroneProcessor final : public juce::AudioProcessor, private juce::Timer, private juce::AudioProcessorValueTreeState::Listener
{
public:
    DroneProcessor();
    ~DroneProcessor() override;
    void prepareToPlay(double, int) override;
    void releaseResources() override { cancelMouseThrough.store(mouseInput.watermark()); cancelMouseRequested.store(true); mouseEpoch.fetch_add(1); }
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return yuro::productName; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 90.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "INIT Drone"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    void flushMidiParameters();
    void selectWaveform(int oscillator, int shape);
    int currentWaveform(int oscillator) const;
    void useAutoPads();
    void learnPads() { padRouter.startLearning(); }
    void setPitchFromMidi(int value);
    void setPitchFromMouse(float cents);
    void setModulation(int value) { modulation.store(juce::jlimit(0, 127, value)); }
    float currentDetune() const;
    static float pitchToCents(int value)
    {
        value = juce::jlimit(0, 16383, value);
        return value <= 8192 ? 100.0f * (value - 8192) / 8192.0f : 100.0f * (value - 8192) / 8191.0f;
    }
    std::atomic<int> modulation { 0 }, pitchValue { 8192 };
    PadRouter padRouter;
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float> peak { 0 };
    std::atomic<int> voices { 0 }, lastCC { -1 }, lastCCValue { 0 };
    std::atomic<int> lastMidiType { 0 }, lastMidiChannel { 0 }, lastNote { -1 };
    std::atomic<int> lastControl { -1 }, lastControlValue { 0 }, lastControlChannel { 0 }, controlHits { 0 };
    std::atomic<int> lastControlCC { -1 };
    std::array<std::atomic<bool>, 128> notes {};
    std::array<std::atomic<float>,128> noteLevels {};
    void randomizeSound(); // message thread only; changes existing host parameters
    yuro::SoundPreset captureSound() const;
    void applySoundPreset(const yuro::SoundPreset&); // message thread, host-notified
    void armFaderPickup(int index) { if(index>=0 && index<4)pickupArmed[static_cast<size_t>(index+6)].store(true); }
    void armEncoderRelative(int index) { if(index>=0 && index<8)encoderRelative[static_cast<size_t>(index<6?index:index+4)].store(true); }
    uint16_t reservedOctaveMask() const;
    bool isReservedNote(int note) const {return note>=0&&note<=127&&(reservedOctaveMask()&(1u<<(note/12)));}
    bool playableView(int first) const;
    int safeKeyboardStart(int preferred) const;
    std::atomic<int> keyboardFirstNote {48};
    std::atomic<bool> panicRequested { false };
    int polyphonyLimit() const { return juce::roundToInt(polyphonyRaw->load()); }
    void setMouseNote(int note) { mouseInput.post(note >= 0 && note <= 127 && !isReservedNote(note) ? note : -1); }
    std::atomic<unsigned> mouseEpoch {0};
private:
    juce::Random inspirationRandom;
    std::array<std::atomic<bool>,12> pickupArmed {};
    std::array<std::atomic<bool>,12> encoderRelative {};
    std::array<int,12> previousControl {};
    juce::RangedAudioParameter* controlParameter(int control) const;
    bool waitForPickup(int control,int value);
    float currentControlNormalized(int control) const;
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    std::array<juce::RangedAudioParameter*, yuro::parameterCount> parameterObjects {};
    std::array<std::atomic<float>*, yuro::parameterCount> raw {};
    // Audio thread writes normalized CC values; message thread notifies host/APVTS.
    std::array<std::atomic<float>, yuro::parameterCount> pending;
    std::array<juce::RangedAudioParameter*, 2> waveObjects {};
    std::array<std::atomic<float>*, 2> waveRaw {};
    std::array<std::atomic<int>, 2> pendingWaves;
    std::array<juce::RangedAudioParameter*, 2> effectObjects {};
    std::array<std::atomic<float>*, 2> effectRaw {};
    std::array<std::atomic<float>, 2> pendingEffects;
    std::atomic<int> pendingPadMode { -1 };
    // Legacy state: channel (0=Any), first note/CC, mode (0=Notes, 1=CC, 2=Auto, 3=Learned).
    std::array<std::atomic<float>*, 4> padConfig {};
    yuro::DroneEngine engine;
    AmbientEffects effects;
    ThreeBandEQ equalizer;
    std::array<juce::RangedAudioParameter*, 3> eqObjects {};
    std::array<std::atomic<float>*, 3> eqRaw {};
    std::array<std::atomic<float>, 3> pendingEQ;
    juce::RangedAudioParameter* pitchParameter = nullptr;
    std::atomic<float>* pitchRaw = nullptr;
    juce::RangedAudioParameter* shimmerParameter = nullptr;
    std::atomic<float>* shimmerRaw = nullptr;
    std::atomic<float> pendingShimmer { -1 };
    std::atomic<float>* polyphonyRaw = nullptr;
    std::array<juce::RangedAudioParameter*,2> sacredParameters {};
    std::array<std::atomic<float>*,2> sacredRaw {};
    std::array<std::atomic<float>,2> pendingSacred;
    void updateSacredTargets();
    MouseNoteInput mouseInput;
    int audioMouseNote = -1;
    std::atomic<bool> cancelMouseRequested {false};
    std::atomic<uint32_t> cancelMouseThrough {0};
    void cancelMouseInput();
    void cancelMouseInputThrough(uint32_t cutoff);
    std::atomic<bool> pitchActive { false }, restoringState { false };
    std::atomic<int> detuneSource { 1 }; // 0 = unchanged legacy ±50 parameter, 1 = new ±100 pitch parameter.
    std::array<float, 3> effectiveEQ() const;
    void parameterChanged(const juce::String&, float) override;
    void updateEffectTargets();
    std::array<float, yuro::parameterCount> effectiveValues() const;
    void timerCallback() override { flushMidiParameters(); }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DroneProcessor)
};
