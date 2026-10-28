// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include "WaveformTests.h"

inline void runControlRoutingTests()
{
    constexpr std::array<const char*, 10> ids { "shimmer", "mix", "attack", "release", "reverb", "delay", "eqBass", "eqMid", "eqTreble", "master" };
    PadRouter router;
    bool protectedRanges = true;
    for (const auto& family : { yuro::controlCC, yuro::dawControlCC }) for (int cc : family)
    {
        const auto message = juce::MidiMessage::controllerEvent(1, cc, 127);
        router.learned[0].store(PadRouter::encode(2, 1, cc));
        protectedRanges = protectedRanges && router.handle(message, 1, 0, 36, cc) == -1
            && router.handle(message, 2, 0, 36, cc) == -1 && router.handle(message, 3, 0, 36, 102) == -1;
    }
    waveCheck(protectedRanges, "Control CCs remain protected from manual, Auto and learned pad mappings");
    DroneProcessor processor; processor.prepareToPlay(48000, 128);
    juce::AudioBuffer<float> audio(2, 128);
    bool allModes = true;
    for (int mode = 0; mode < 4; ++mode)
    {
        setPlain(processor, "padInput", static_cast<float>(mode)); setPlain(processor, "padChannel", 10); setPlain(processor, "padFirstCC", 70);
        for (const auto& family : { yuro::controlCC, yuro::dawControlCC }) for (int channel = 1; channel <= 16; ++channel)
            for (size_t i = 0; i < ids.size(); ++i)
            {
                processor.padRouter.learned[i % 8].store(PadRouter::encode(2, channel, family[i]));
                const int value = (17 + static_cast<int>(i) * 13 + channel * 7 + mode * 11) % 128;
                juce::MidiBuffer midi; midi.addEvent(juce::MidiMessage::controllerEvent(channel, family[i], value), 31);
                processor.processBlock(audio, midi); processor.flushMidiParameters();
                const float expected = i >= 6 && i <= 8 ? ThreeBandEQ::ccToDb(value)
                    : processor.parameters.getParameter(ids[i])->convertFrom0to1(static_cast<float>(value) / 127);
                allModes = allModes && std::abs(processor.parameters.getRawParameterValue(ids[i])->load() - expected) < 0.011f
                    && processor.lastControl.load() == static_cast<int>(i) && processor.lastControlChannel.load() == channel;
            }
    }
    waveCheck(allModes && processor.controlHits.load() == 1280, "E1-E6 and F1-F4 work in both CC families, four pad modes and sixteen channels");
    const float legacy = processor.parameters.getRawParameterValue("detune")->load();
    for (int cc : { 74, 86 })
    {
        juce::MidiBuffer midi; midi.addEvent(juce::MidiMessage::controllerEvent(1, cc, 127), 0); processor.processBlock(audio, midi);
    }
    processor.flushMidiParameters();
    waveCheck(processor.parameters.getRawParameterValue("detune")->load() == legacy && processor.currentDetune() == 0 && processor.parameters.getRawParameterValue("shimmer")->load() == 1,
        "E1 CC74 and CC86 drive Shimmer while leaving both detune parameters alone");
    processor.learnPads();
    for (size_t i = 0; i < ids.size(); ++i)
    {
        juce::MidiBuffer midi; midi.addEvent(juce::MidiMessage::controllerEvent(3, yuro::controlCC[i], 90), 0); processor.processBlock(audio, midi);
    }
    processor.flushMidiParameters();
    waveCheck(processor.padRouter.learningStatus() == 0, "Internal legacy pad-learning state does not block the ten active CC controls");
    processor.useAutoPads();
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 38, 0.8f), 0);
    for (size_t i = 0; i < ids.size(); ++i) midi.addEvent(juce::MidiMessage::controllerEvent(1, yuro::dawControlCC[i], 64), static_cast<int>(i));
    midi.addEvent(juce::MidiMessage::noteOff(1, 38), 10); midi.addEvent(juce::MidiMessage::controllerEvent(1, 109, 127), 11);
    processor.processBlock(audio, midi); processor.flushMidiParameters();
    waveCheck(processor.currentWaveform(0) == 2 && processor.currentWaveform(1) == 3 && processor.voices.load() == 0,
        "Pad notes, pad CC, releases, synthesis controls and EQ coexist in one buffer");
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    bool attached = true, mouse = true;
    for (size_t i = 0; i < ids.size(); ++i)
    {
        const auto name = i < 6 ? "Encoder " + juce::String(static_cast<int>(i + 1)) : "Fader " + juce::String(static_cast<int>(i - 5));
        juce::Slider* slider = nullptr;
        for (auto* child : editor->getChildren()) if (child->getName() == name) slider = dynamic_cast<juce::Slider*>(child);
        attached = attached && slider && std::abs(static_cast<float>(slider->getValue()) - processor.parameters.getRawParameterValue(ids[i])->load()) < 0.011f;
        if (slider)
        {
            const float target = processor.parameters.getParameter(ids[i])->convertFrom0to1(0.37f);
            slider->setValue(target, juce::sendNotificationSync);
            mouse = mouse && std::abs(processor.parameters.getRawParameterValue(ids[i])->load() - target) < 0.011f;
        }
        else mouse = false;
    }
    waveCheck(attached, "Hardware CC updates are reflected by E2-E6 and all four GUI faders");
    waveCheck(mouse && processor.currentWaveform(0) == 2 && processor.currentWaveform(1) == 3,
        "Mouse edits of synthesis, effects and EQ preserve pad choices");
    juce::MemoryBlock state; processor.getStateInformation(state);
    DroneProcessor restored; restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    bool saved = true;
    for (size_t i = 0; i < ids.size(); ++i)
        saved = saved && std::abs(restored.parameters.getRawParameterValue(ids[i])->load() - processor.parameters.getRawParameterValue(ids[i])->load()) < 0.011f
            && restored.parameters.getParameter(ids[i])->isAutomatable();
    waveCheck(saved && restored.currentWaveform(0) == 2 && restored.currentWaveform(1) == 3, "All ten CC parameters, EQ and waveforms retain host state and automation");
}
