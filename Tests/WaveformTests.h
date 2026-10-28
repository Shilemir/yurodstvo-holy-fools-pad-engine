// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include "../Source/PluginProcessor.h"
#include <iostream>
#include <stdexcept>
#include <memory>
#include <vector>

inline void waveCheck(bool success, const char* name)
{
    if (!success) throw std::runtime_error(name);
    std::cout << "PASS: " << name << std::endl;
}
inline void setPlain(DroneProcessor& processor, const char* id, float value)
{
    auto* p = processor.parameters.getParameter(id);
    p->setValueNotifyingHost(p->convertTo0to1(value));
}
inline void runWaveformTests()
{
    auto tables = std::make_unique<yuro::WaveTables>();
    constexpr int count = 8192;
    constexpr double twoPi = 6.28318530717958647692;
    auto project = [&](const std::vector<float>& values, int bin)
    {
        double s = 0, c = 0;
        for (int i = 0; i < count; ++i)
        {
            const double angle = twoPi * bin * i / count;
            s += values[static_cast<size_t>(i)] * std::sin(angle);
            c += values[static_cast<size_t>(i)] * std::cos(angle);
        }
        return std::array<double, 2> { 2 * s / count, 2 * c / count };
    };
    auto magnitude = [](std::array<double, 2> pair) { return std::hypot(pair[0], pair[1]); };
    for (int shape = 0; shape < 4; ++shape)
    {
        std::vector<float> values(count);
        for (int i = 0; i < count; ++i)
            values[static_cast<size_t>(i)] = tables->sample(shape, static_cast<double>((i * 32) % count) / count, 32.0 / count);
        const auto fundamental = magnitude(project(values, 32));
        const auto second = magnitude(project(values, 64)) / fundamental;
        const auto third = magnitude(project(values, 96)) / fundamental;
        waveCheck(fundamental > 0.3, "Every waveform has a usable fundamental");
        if (shape == 0) waveCheck(second < 0.0001 && third < 0.0001, "Sine preserves its pure spectrum");
        if (shape == 1) waveCheck(second < 0.0001 && std::abs(third - 1.0 / 9) < 0.001, "Triangle has odd 1/n^2 harmonics");
        if (shape == 2) waveCheck(std::abs(second - 0.5) < 0.001 && std::abs(third - 1.0 / 3) < 0.001, "Sawtooth has 1/n harmonics");
        if (shape == 3) waveCheck(second < 0.0001 && std::abs(third - 1.0 / 3) < 0.001, "Square has odd 1/n harmonics");
        for (int i = 0; i < count; ++i)
            values[static_cast<size_t>(i)] = tables->sample(shape, static_cast<double>((i * 1403) % count) / count, 1403.0 / count);
        const auto a = project(values, 1403), b = project(values, 2806);
        double residual = 0, total = 0;
        for (int i = 0; i < count; ++i)
        {
            const double phase = twoPi * i / count;
            const double expected = a[0] * std::sin(1403 * phase) + a[1] * std::cos(1403 * phase)
                                  + b[0] * std::sin(2806 * phase) + b[1] * std::cos(2806 * phase);
            const double error = values[static_cast<size_t>(i)] - expected;
            residual += error * error; total += values[static_cast<size_t>(i)] * values[static_cast<size_t>(i)];
        }
        waveCheck(residual / total < 1.0e-10, "High-note spectrum contains only safe harmonics (residual below -100 dB)");
    }
    // Sweep table boundaries and frequencies, including rapidly changing detune.
    bool finiteAndBounded = true;
    for (int shape = 0; shape < 4; ++shape)
        for (int i = 1; i < 40000; ++i)
        {
            const double dt = 0.0001 + 0.48 * i / 40000;
            const float value = tables->sample(shape, std::fmod(i * 0.013719, 1.0), dt);
            finiteAndBounded = finiteAndBounded && std::isfinite(value) && std::abs(value) <= 1.15001f;
        }
    waveCheck(finiteAndBounded, "All table banks and frequency transitions respect the oscillator peak budget");
    yuro::DroneEngine reference, morph;
    for (auto* e : { &reference, &morph })
    {
        e->prepare(48000); e->setTargets({ 0, 0, 0.005f, 5, 1 });
        for (int i = 0; i < 4800; ++i) e->nextSample();
        e->noteOn(1, 48, 1);
        for (int i = 0; i < 4800; ++i) e->nextSample();
    }
    morph.setWaveforms({ 2, 0 });
    const float first = morph.nextSample(), baseline = reference.nextSample();
    waveCheck(std::abs(first - baseline) < 0.0003f, "Waveform switching starts with a short continuous crossfade");
    double difference = 0; float last = first, largestStep = 0;
    for (int i = 0; i < 4800; ++i)
    {
        const float sample = morph.nextSample();
        const float delta = sample - reference.nextSample();
        if (i > 2400) difference += delta * delta;
        largestStep = std::max(largestStep, std::abs(sample - last)); last = sample;
    }
    waveCheck(difference > 0.01 && morph.activeVoices() == 1, "Held note changes timbre without retriggering");
    waveCheck(largestStep < 0.1f, "Morph has no large sample discontinuity");
    morph.noteOff(1, 48);
    bool safeMorph = true;
    for (int i = 0; i < 20000; ++i)
    {
        if (i % 97 == 0) morph.setWaveforms({ (i / 97) % 4, (i / 291) % 4 });
        const auto sample = morph.nextSample();
        safeMorph = safeMorph && std::isfinite(sample) && std::abs(sample) <= 0.940001f;
    }
    waveCheck(safeMorph && morph.activeVoices() == 1, "Rapid repeated switches also affect release tails safely");
    yuro::DroneEngine isolatedA, isolatedB;
    for (auto* e : { &isolatedA, &isolatedB })
    {
        e->prepare(48000); e->setTargets({ 23, 0, 0.005f, 5, 1 });
        for (int i = 0; i < 4800; ++i) e->nextSample();
        e->noteOn(1, 57, 1);
    }
    isolatedA.setWaveforms({ 2, 1 }); isolatedB.setWaveforms({ 2, 3 });
    bool independent = true;
    for (int i = 0; i < 4800; ++i) independent = independent && isolatedA.nextSample() == isolatedB.nextSample();
    waveCheck(independent, "Mix zero isolates oscillator 1 regardless of oscillator 2 waveform");
    isolatedA.setTargets({ 23, 1, 0.005f, 5, 1 }); isolatedB.setTargets({ 23, 1, 0.005f, 5, 1 });
    isolatedA.setWaveforms({ 0, 1 }); isolatedB.setWaveforms({ 3, 1 });
    for (int i = 0; i < 4800; ++i) { isolatedA.nextSample(); isolatedB.nextSample(); }
    independent = true;
    for (int i = 0; i < 4800; ++i) independent = independent && isolatedA.nextSample() == isolatedB.nextSample();
    waveCheck(independent, "Mix one isolates the detuned oscillator 2 regardless of oscillator 1 waveform");
    yuro::DroneEngine stress;
    stress.prepare(48000); stress.setTargets({ 50, 0.5f, 0.005f, 30, 1 });
    for (int i = 0; i < 4800; ++i) stress.nextSample();
    for (int i = 0; i < 8; ++i) stress.noteOn(1, 48 + i * 7, 1);
    const auto start = juce::Time::getMillisecondCounterHiRes();
    bool safePolyphony = true;
    for (int i = 0; i < 48000; ++i)
    {
        if (i % 127 == 0) stress.setWaveforms({ (i / 127) % 4, (i / 508) % 4 });
        const auto sample = stress.nextSample();
        safePolyphony = safePolyphony && std::isfinite(sample) && std::abs(sample) <= 0.940001f;
    }
    const auto duration = juce::Time::getMillisecondCounterHiRes() - start;
    std::cout << "Eight-voice rapid-morph render: " << duration << " ms per 1000 ms audio" << std::endl;
    waveCheck(safePolyphony && stress.activeVoices() == 8, "All sixteen waveform pairs render safely under eight-voice morph stress");

    DroneProcessor processor;
    processor.prepareToPlay(48000, 128);
    juce::AudioBuffer<float> audio(2, 128);
    auto send = [&](juce::MidiMessage event)
    {
        juce::MidiBuffer midi; midi.addEvent(event, 0); processor.processBlock(audio, midi);
    };
    waveCheck(processor.currentWaveform(0) == 0 && processor.currentWaveform(1) == 0, "Default waveform is Sine + Sine");
    setPlain(processor, "padInput", 0); setPlain(processor, "padChannel", 10);
    for (int pad = 0; pad < 8; ++pad)
    {
        send(juce::MidiMessage::noteOn(10, 36 + pad, 0.9f));
        waveCheck(processor.currentWaveform(pad / 4) == pad % 4 && processor.voices.load() == 0, "Each Bank A pad selects a shape without starting a voice");
        send(juce::MidiMessage::noteOff(10, 36 + pad));
        waveCheck(processor.currentWaveform(pad / 4) == pad % 4, "Pad release preserves the selected waveform");
    }
    send(juce::MidiMessage::noteOn(1, 36, 1.0f));
    waveCheck(processor.voices.load() == 0, "The complete pad-control octave is silent on every MIDI channel");
    send(juce::MidiMessage::noteOn(10, 36, 0.0f));
    waveCheck(processor.currentWaveform(0) == 3, "Zero-velocity Note On is a release, not a new selection");
    send(juce::MidiMessage::noteOff(1, 36));
    setPlain(processor, "padChannel", 0); setPlain(processor, "padFirstNote", 44);
    send(juce::MidiMessage::noteOn(1, 46, 1.0f));
    waveCheck(processor.currentWaveform(0) == 2 && processor.lastMidiChannel.load() == 1 && processor.lastNote.load() == 46,
        "Configurable note base and Any channel support host rerouting with diagnostics");
    setPlain(processor, "padInput", 1);
    send(juce::MidiMessage::controllerEvent(1, 108, 127));
    waveCheck(processor.currentWaveform(1) == 2, "CC pad mode selects oscillator 2 independently");
    send(juce::MidiMessage::controllerEvent(1, 108, 0));
    waveCheck(processor.currentWaveform(1) == 2, "CC pad release does not undo selection");
    send(juce::MidiMessage::controllerEvent(1, 71, 127));
    processor.flushMidiParameters();
    waveCheck(processor.parameters.getRawParameterValue("mix")->load() == 1, "Encoder CC mapping coexists with pad CC mapping");
    send(juce::MidiMessage::controllerEvent(1, 109, 127));
    juce::MemoryBlock state; processor.getStateInformation(state);
    DroneProcessor restored; restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    waveCheck(restored.currentWaveform(0) == 2 && restored.currentWaveform(1) == 3, "Both waveforms survive save before the UI timer flushes");
    waveCheck(restored.parameters.getRawParameterValue("padChannel")->load() == 0
        && restored.parameters.getRawParameterValue("padFirstNote")->load() == 44
        && restored.parameters.getRawParameterValue("padInput")->load() == 1, "Pad mapping is restored with the host state");
    setPlain(restored, "osc1Waveform", 1);
    waveCheck(restored.currentWaveform(0) == 1 && restored.parameters.getParameter("osc1Waveform")->isAutomatable(), "Waveform choice is an automatable host parameter");
    juce::ValueTree oldState("YURODSTVO_STATE");
    constexpr std::array<float, 5> oldValues { 11, 0.2f, 12, 23, 0.41f };
    for (size_t i = 0; i < oldValues.size(); ++i)
    {
        juce::ValueTree child("PARAM"); child.setProperty("id", yuro::parameterIds[i], nullptr);
        child.setProperty("value", oldValues[i], nullptr); oldState.appendChild(child, nullptr);
    }
    juce::MemoryBlock oldBinary; auto oldXml = oldState.createXml(); juce::AudioProcessor::copyXmlToBinary(*oldXml, oldBinary);
    restored.setStateInformation(oldBinary.getData(), static_cast<int>(oldBinary.getSize()));
    bool originalValues = true;
    for (size_t i = 0; i < oldValues.size(); ++i)
        originalValues = originalValues && std::abs(restored.parameters.getRawParameterValue(yuro::parameterIds[i])->load()
            - (i == 3 ? yuro::maxEnvelopeSeconds : oldValues[i])) < 0.002f;
    waveCheck(originalValues && restored.currentWaveform(0) == 0 && restored.currentWaveform(1) == 0,
        "Legacy five-parameter state migrates to Sine + Sine, preserving values except the new Release cap");
}
