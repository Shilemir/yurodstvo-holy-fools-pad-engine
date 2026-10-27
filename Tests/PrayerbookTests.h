#pragma once
#include "WaveformTests.h"
#include <juce_audio_devices/juce_audio_devices.h>

inline void runPrayerbookTests()
{
    DroneProcessor p;
    p.prepareToPlay(48000, 256);
    juce::AudioBuffer<float> audio(2, 256);
    auto send = [&](juce::MidiMessage message)
    {
        juce::MidiBuffer midi; midi.addEvent(message, 12); p.processBlock(audio, midi);
    };
    waveCheck(p.getName() == "YURODSTVO - Holy Fool's Pad Engine", "Instrument reports the Holy Fool name");
    waveCheck(p.parameters.getRawParameterValue("padInput")->load() == 2
        && p.parameters.getRawParameterValue("padChannel")->load() == 0, "New instances start in MiniLab Auto on any channel");
    send(juce::MidiMessage::noteOn(1, 38, 1.0f));
    waveCheck(p.currentWaveform(0) == 2 && p.voices.load() == 0, "Auto mode accepts Live-style channel 1 pads without starting notes");
    send(juce::MidiMessage::noteOff(1, 38));
    waveCheck(p.currentWaveform(0) == 2, "Auto pad Note Off preserves selection");
    send(juce::MidiMessage::controllerEvent(10, 109, 1));
    waveCheck(p.currentWaveform(1) == 3 && p.padRouter.lastPad.load() == 7, "Auto mode also accepts CC pads and positive gate value 1");
    send(juce::MidiMessage::controllerEvent(10, 109, 0));
    waveCheck(p.currentWaveform(1) == 3, "CC zero is a release and does not undo a waveform");
    send(juce::MidiMessage::noteOn(1, 60, 0.8f));
    waveCheck(p.voices.load() == 1, "Normal keyboard notes coexist with Auto pad routing");
    waveCheck(p.padRouter.lastNumber.load() == 109 && p.lastNote.load() == 60, "Matched-pad diagnostics remain visible after keyboard messages");
    p.learnPads();
    for (int pad = 0; pad < 8; ++pad)
    {
        auto event = pad % 2 == 0 ? juce::MidiMessage::noteOn(1, 80 + pad, 0.9f)
                                 : juce::MidiMessage::controllerEvent(3, 50 + pad, 127);
        send(event); send(event); // Duplicate pressure/hits must not advance learning twice.
        waveCheck(p.padRouter.learningStep.load() == pad + 1, "Pad learning advances once per distinct mapping");
    }
    waveCheck(p.padRouter.learned[7].load() == PadRouter::encode(2, 3, 57), "Learning captures actual Note/CC number and channel");
    juce::MemoryBlock learnedState; p.getStateInformation(learnedState);
    DroneProcessor restored;
    restored.setStateInformation(learnedState.getData(), static_cast<int>(learnedState.getSize()));
    restored.prepareToPlay(48000, 256);
    waveCheck(restored.parameters.getRawParameterValue("padInput")->load() == 3, "Learned mode is saved even before the host timer flush");
    juce::MidiBuffer learnedMidi; learnedMidi.addEvent(juce::MidiMessage::noteOn(1, 82, 1.0f), 0);
    restored.processBlock(audio, learnedMidi);
    waveCheck(restored.currentWaveform(0) == 2 && restored.voices.load() == 0, "Restored custom pad map selects waveforms and consumes pad notes");
    learnedMidi.addEvent(juce::MidiMessage::controllerEvent(3, 57, 127), 0);
    restored.processBlock(audio, learnedMidi);
    waveCheck(restored.currentWaveform(1) == 3, "Mixed Note and CC learned pads control oscillator 2");
    restored.useAutoPads();
    waveCheck(restored.parameters.getRawParameterValue("padInput")->load() == 2, "One-click MiniLab Auto restores the standard routing");

    // Recreate a five-parameter-plus-pads 0.3 factory state with its channel-10 trap.
    auto legacy = p.parameters.copyState();
    legacy.removeProperty("schemaVersion", nullptr);
    legacy.getChildWithProperty("id", "padInput").setProperty("value", 0, nullptr);
    legacy.getChildWithProperty("id", "padChannel").setProperty("value", 10, nullptr);
    legacy.getChildWithProperty("id", "padFirstNote").setProperty("value", 36, nullptr);
    legacy.getChildWithProperty("id", "padFirstCC").setProperty("value", 102, nullptr);
    legacy.removeChild(legacy.getChildWithProperty("id", "reverb"), nullptr);
    legacy.removeChild(legacy.getChildWithProperty("id", "delay"), nullptr);
    auto xml = legacy.createXml(); juce::MemoryBlock legacyData;
    juce::AudioProcessor::copyXmlToBinary(*xml, legacyData);
    restored.setStateInformation(legacyData.getData(), static_cast<int>(legacyData.getSize()));
    waveCheck(restored.parameters.getRawParameterValue("padInput")->load() == 2
        && restored.parameters.getRawParameterValue("padChannel")->load() == 0, "Legacy factory routing migrates from strict channel 10 to Auto");
    waveCheck(restored.parameters.getRawParameterValue("reverb")->load() == 0
        && restored.parameters.getRawParameterValue("delay")->load() == 0, "Old projects restore with effects off");
    send(juce::MidiMessage::controllerEvent(1, 93, 96));
    send(juce::MidiMessage::controllerEvent(1, 18, 72));
    juce::MemoryBlock fxState; p.getStateInformation(fxState);
    restored.setStateInformation(fxState.getData(), static_cast<int>(fxState.getSize()));
    waveCheck(std::abs(restored.parameters.getRawParameterValue("reverb")->load() - 96.0f / 127) < 0.001f
        && std::abs(restored.parameters.getRawParameterValue("delay")->load() - 72.0f / 127) < 0.001f,
        "E5 CC93 and E6 CC18 are saved before the UI timer flush");
    p.flushMidiParameters();
    waveCheck(p.parameters.getParameter("reverb")->isAutomatable() && p.parameters.getParameter("delay")->isAutomatable(), "Both effect macros are automatable host parameters");
    for (double sr : { 44100.0, 48000.0, 96000.0 })
    {
        AmbientEffects fx; fx.prepare(sr, 1); fx.setTargets(0, 0, 1);
        bool dry = true;
        for (int i = 0; i < 2048; ++i)
        {
            const float input = 0.2f * std::sin(i * 0.02f);
            const auto output = fx.process(input);
            dry = dry && output[0] == input && output[1] == input;
        }
        waveCheck(dry, "Effects at zero preserve dry stereo samples exactly");
        fx.prepare(sr, 1); fx.setTargets(0, 1, 1);
        for (int i = 0; i < static_cast<int>(sr / 5); ++i) fx.process(0);
        fx.process(0.25f);
        float leftEcho = 0, rightEcho = 0;
        for (int i = 1; i < static_cast<int>(sr * 0.9); ++i)
        {
            const auto out = fx.process(0);
            if (i >= static_cast<int>(sr * 0.56) && i <= static_cast<int>(sr * 0.58)) leftEcho = std::max(leftEcho, std::abs(out[0]));
            if (i >= static_cast<int>(sr * 0.75) && i <= static_cast<int>(sr * 0.77)) rightEcho = std::max(rightEcho, std::abs(out[1]));
        }
        waveCheck(leftEcho > 0.05f && rightEcho > 0.05f, "Delay impulse produces stereo echoes at 570 and 760 ms");
        fx.prepare(sr, 1); fx.setTargets(1, 0, 1);
        for (int i = 0; i < static_cast<int>(sr / 5); ++i) fx.process(0);
        fx.process(0.25f);
        double tail = 0, lateTail = 0;
        for (int i = 0; i < static_cast<int>(sr * 12); ++i)
        {
            const auto out = fx.process(0);
            if (i > sr * 0.1 && i < sr) tail += out[0] * out[0] + out[1] * out[1];
            if (i > sr * 11) lateTail += out[0] * out[0] + out[1] * out[1];
        }
        waveCheck(tail > 0.00001 && lateTail < tail * 0.01, "Reverb creates a diffuse tail which decays safely");
        fx.setTargets(1, 1, 1);
        bool bounded = true; float previous = 0, maxStep = 0;
        for (int i = 0; i < static_cast<int>(sr * 2); ++i)
        {
            if (i % 512 == 0) fx.setTargets((i / 512) % 2 ? 0.0f : 1.0f, (i / 512) % 2 ? 1.0f : 0.0f, 1);
            const auto out = fx.process(0.2f * std::sin(static_cast<float>(i * 220 * 6.283185307 / sr)));
            bounded = bounded && std::isfinite(out[0]) && std::isfinite(out[1]) && std::abs(out[0]) <= 0.940001f && std::abs(out[1]) <= 0.940001f;
            maxStep = std::max(maxStep, std::abs(out[0] - previous)); previous = out[0];
        }
        waveCheck(bounded && maxStep < 0.1f, "Rapid effect automation is bounded and free of large discontinuities");
        fx.setTargets(1, 1, 0);
        for (int i = 0; i < static_cast<int>(sr / 5); ++i) fx.process(0);
        auto silent = fx.process(0);
        waveCheck(silent[0] == 0 && silent[1] == 0, "Master zero mutes effect tails as well as dry audio");
        fx.setTargets(1, 1, 1); fx.beginPanic();
        for (int i = 0; i < static_cast<int>(sr / 5); ++i) fx.process(0);
        silent = fx.process(0);
        waveCheck(silent[0] == 0 && silent[1] == 0, "Panic fades and clears delay and reverb tails");
    }
    DroneProcessor full;
    setPlain(full, "reverb", 1); setPlain(full, "delay", 1); setPlain(full, "attack", 0.005f);
    full.prepareToPlay(48000, 256);
    juce::MidiBuffer chord;
    for (int note = 48; note < 88; note += 5) chord.addEvent(juce::MidiMessage::noteOn(1, note, 1.0f), 0);
    bool safe = true;
    const auto fxStart = juce::Time::getMillisecondCounterHiRes();
    for (int block = 0; block < 400; ++block)
    {
        if (block % 8 == 0) { full.selectWaveform(0, (block / 8) % 4); full.selectWaveform(1, (block / 32) % 4); }
        full.processBlock(audio, chord); chord.clear();
        for (int ch = 0; ch < 2; ++ch) for (int i = 0; i < audio.getNumSamples(); ++i)
            safe = safe && std::isfinite(audio.getSample(ch, i)) && std::abs(audio.getSample(ch, i)) <= 0.940001f;
    }
    std::cout << "Eight-voice full-FX processor: " << juce::Time::getMillisecondCounterHiRes() - fxStart << " ms per 2133 ms audio" << std::endl;
    waveCheck(safe && full.voices.load() == 8, "Full processor is safe with eight voices, rapid wave changes and both effects at maximum");
    setPlain(full, "master", 0);
    for (int block = 0; block < 20; ++block) full.processBlock(audio, chord);
    waveCheck(audio.getMagnitude(0, audio.getNumSamples()) == 0, "Processor Master mutes sustained voices and all effect tails");
    setPlain(full, "master", 1); full.panicRequested.store(true);
    for (int block = 0; block < 20; ++block) full.processBlock(audio, chord);
    waveCheck(full.voices.load() == 0 && audio.getMagnitude(0, audio.getNumSamples()) == 0, "Processor Panic clears voices and effects without a residual tail");
    std::cout << "Available MiniLab MIDI inputs:" << std::endl;
    bool found = false;
    for (const auto& device : juce::MidiInput::getAvailableDevices())
        if (device.name.containsIgnoreCase("minilab")) { found = true; std::cout << device.name << std::endl; }
    if (!found) std::cout << "No MiniLab MIDI input enumerated in this test process." << std::endl;
}
