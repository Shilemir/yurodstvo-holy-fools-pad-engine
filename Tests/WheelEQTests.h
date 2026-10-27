#pragma once
#include "WaveformTests.h"
#include "../Source/PluginEditor.h"

inline void runWheelEQTests()
{
    waveCheck(DroneProcessor::pitchToCents(0) == -100 && DroneProcessor::pitchToCents(8192) == 0 && DroneProcessor::pitchToCents(16383) == 100,
        "14-bit Pitch Bend has exact -100, 0 and +100 cent endpoints");
    waveCheck(DroneProcessor::pitchToCents(8191) < 0 && DroneProcessor::pitchToCents(8193) > 0
        && std::abs(DroneProcessor::pitchToCents(8193)) < 0.013f, "Pitch Bend retains 14-bit resolution around centre");
    waveCheck(ThreeBandEQ::ccToDb(0) == -12 && ThreeBandEQ::ccToDb(64) == 0 && ThreeBandEQ::ccToDb(127) == 12,
        "EQ CC endpoints and physical centre 64 map to -12, 0 and +12 dB");
    DroneProcessor p; p.prepareToPlay(48000, 256); juce::AudioBuffer<float> audio(2, 256);
    auto send = [&](juce::MidiMessage message) { juce::MidiBuffer m; m.addEvent(message, 0); p.processBlock(audio, m); };
    waveCheck(p.currentDetune() == 0 && p.modulation.load() == 0 && p.parameters.getRawParameterValue("eqBass")->load() == 0
        && p.parameters.getRawParameterValue("eqMid")->load() == 0 && p.parameters.getRawParameterValue("eqTreble")->load() == 0,
        "New 0.6 instances start with centred pitch and neutral three-band EQ");
    setPlain(p, "pitchDetune", 23);
    send(juce::MidiMessage::pitchWheel(1, 16383));
    waveCheck(p.currentDetune() == 100, "Physical Pitch Bend overrides detune in the sounding instrument");
    send(juce::MidiMessage::controllerEvent(1, 1, 127));
    waveCheck(p.modulation.load() == 127 && p.currentDetune() == 100, "CC1 updates Mod position without changing detune");
    juce::MemoryBlock saved; p.getStateInformation(saved);
    DroneProcessor restored; restored.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    waveCheck(restored.currentDetune() == 23 && restored.pitchValue.load() == 8192 && restored.modulation.load() == 0,
        "Saving while Pitch is held preserves base automation but excludes temporary Pitch and Mod positions");
    restored.prepareToPlay(48000, 256);
    waveCheck(restored.currentDetune() == 23 && restored.modulation.load() == 0,
        "Reinitializing audio preserves saved pitch automation while resetting transient controller positions");
    send(juce::MidiMessage::pitchWheel(1, 8192));
    waveCheck(p.currentDetune() == 0, "Releasing physical Pitch Bend to centre gives exactly zero detune");
    setPlain(p, "pitchDetune", -42);
    waveCheck(p.currentDetune() == -42, "New host pitch automation takes over cleanly from a centred temporary gesture");
    const auto range = p.parameters.getParameter("detune")->getNormalisableRange();
    waveCheck(range.start == -50 && range.end == 50, "Legacy Detune ID and normalized automation range remain unchanged");
    auto legacy = p.parameters.copyState(); legacy.removeProperty("schemaVersion", nullptr);
    legacy.removeChild(legacy.getChildWithProperty("id", "pitchDetune"), nullptr);
    for (const auto* id : { "eqBass", "eqMid", "eqTreble" })
    {
        legacy.removeChild(legacy.getChildWithProperty("id", id), nullptr);
        setPlain(restored, id, 9);
    }
    legacy.getChildWithProperty("id", "detune").setProperty("value", 25, nullptr);
    auto xml = legacy.createXml(); juce::MemoryBlock old; juce::AudioProcessor::copyXmlToBinary(*xml, old);
    restored.setStateInformation(old.getData(), static_cast<int>(old.getSize()));
    waveCheck(restored.currentDetune() == 25, "Old saved Detune values migrate without doubling or discarding their cents");
    waveCheck(restored.parameters.getRawParameterValue("eqBass")->load() == 0
        && restored.parameters.getRawParameterValue("eqMid")->load() == 0 && restored.parameters.getRawParameterValue("eqTreble")->load() == 0,
        "Loading an old state into a reused instance resets all missing EQ bands to neutral");
    restored.setPitchFromMidi(8192);
    waveCheck(restored.currentDetune() == 0, "Even a legacy preset returns to zero on a physical centre message");
    restored.getStateInformation(saved); DroneProcessor oldAgain;
    oldAgain.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    waveCheck(oldAgain.currentDetune() == 25, "Transient wheel centring does not overwrite the legacy preset base");
    setPlain(p, "attack", 0.005f); setPlain(p, "mix", 1); setPlain(p, "master", 1); setPlain(p, "pitchDetune", 0);
    p.prepareToPlay(48000, 256); send(juce::MidiMessage::noteOn(1, 69, 1.0f));
    auto advance = [&](int blocks) { for (int i = 0; i < blocks; ++i) { juce::MidiBuffer m; p.processBlock(audio, m); } };
    auto crossings = [&]
    {
        int result = 0, remaining = 48000; float before = 0;
        while (remaining > 0)
        {
            juce::MidiBuffer m; p.processBlock(audio, m);
            const int n = juce::jmin(remaining, audio.getNumSamples());
            for (int i = 0; i < n; ++i) { const float sample = audio.getSample(0, i); if (before <= 0 && sample > 0) ++result; before = sample; }
            remaining -= n;
        }
        return result;
    };
    advance(40); send(juce::MidiMessage::pitchWheel(1, 16383)); advance(40);
    waveCheck(std::abs(crossings() - 466) <= 1 && p.voices.load() == 1, "Pitch +100 changes a held oscillator II to about 466 Hz without retrigger");
    send(juce::MidiMessage::pitchWheel(1, 0)); advance(40);
    waveCheck(std::abs(crossings() - 415) <= 1, "Pitch -100 changes oscillator II to about 415 Hz");
    setPlain(p, "mix", 0); send(juce::MidiMessage::pitchWheel(1, 16383)); advance(40);
    waveCheck(std::abs(crossings() - 440) <= 1, "Pitch Wheel does not globally bend oscillator I");
    for (double sr : { 44100.0, 48000.0, 96000.0 })
    {
        ThreeBandEQ flat; flat.prepare(sr);
        bool neutral = true, isolated = true;
        for (int i = 0; i < 2000; ++i)
        {
            const float x = 0.1f * std::sin(i * 0.02f); const auto out = flat.process({ x, 0 });
            neutral = neutral && std::abs(out[0] - x) < 0.000001f; isolated = isolated && out[1] == 0;
        }
        waveCheck(neutral && isolated, "Neutral EQ is transparent and stereo channel states are isolated");
        for (int band = 0; band < 3; ++band) for (float db : { -12.0f, 12.0f })
        {
            ThreeBandEQ eq; std::array<float, 3> gains {}; gains[static_cast<size_t>(band)] = db; eq.prepare(sr, gains);
            const double f = band == 0 ? 20 : band == 1 ? 1000 : sr * 0.40;
            const int n = static_cast<int>(sr * 0.4); double inEnergy = 0, outEnergy = 0;
            for (int i = 0; i < n; ++i)
            {
                const float x = 0.01f * static_cast<float>(std::sin(juce::MathConstants<double>::twoPi * f * i / sr));
                const auto out = eq.process({ x, x });
                if (i >= n / 2) { inEnergy += x * x; outEnergy += out[0] * out[0]; }
            }
            const double measured = 20 * std::log10(std::sqrt(outEnergy / inEnergy));
            waveCheck(std::abs(measured - db) < 0.65, "Measured shelf/peak response matches requested +/-12 dB with no global compensation");
        }
        ThreeBandEQ moving; moving.prepare(sr); bool stable = true; float previous = 0, maxStep = 0;
        for (int i = 0; i < static_cast<int>(sr); ++i)
        {
            if (i % 97 == 0) moving.setTargets({ (i / 97) % 2 ? -12.0f : 12.0f, (i / 194) % 2 ? 12.0f : -12.0f, (i / 291) % 2 ? 12.0f : -12.0f });
            const auto out = moving.process({ 0.2f * static_cast<float>(std::sin(i * 500 * juce::MathConstants<double>::twoPi / sr)), 0 });
            stable = stable && std::isfinite(out[0]) && out[1] == 0 && std::abs(out[0]) < 2;
            maxStep = std::max(maxStep, std::abs(out[0] - previous)); previous = out[0];
        }
        waveCheck(stable && maxStep < 0.1f, "Rapid three-band EQ automation is stable and has no large discontinuities");
    }
    DroneProcessor a, b; a.prepareToPlay(48000, 256); b.prepareToPlay(48000, 256);
    juce::AudioBuffer<float> other(2, 256); bool noModSound = true;
    for (int block = 0; block < 12; ++block)
    {
        juce::MidiBuffer ma, mb;
        if (block == 0) { ma.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 0); mb = ma; mb.addEvent(juce::MidiMessage::controllerEvent(1, 1, 127), 0); }
        a.processBlock(audio, ma); b.processBlock(other, mb);
        for (int i = 0; i < 256; ++i) noModSound = noModSound && audio.getSample(0, i) == other.getSample(0, i);
    }
    waveCheck(noModSound && b.modulation.load() == 127, "Mod Wheel is visible state only and has no audio destination");
    auto editor = std::unique_ptr<juce::AudioProcessorEditor>(b.createEditor());
    juce::Slider* pitch = nullptr; juce::Slider* mod = nullptr; juce::TextButton* panic = nullptr;
    for (auto* child : editor->getChildren())
    {
        if (child->getName() == "Pitch Wheel") pitch = dynamic_cast<juce::Slider*>(child);
        if (child->getName() == "Mod Wheel") mod = dynamic_cast<juce::Slider*>(child);
        if (auto* button = dynamic_cast<juce::TextButton*>(child)) panic = button;
    }
    waveCheck(pitch && mod && mod->getValue() == 127, "Both on-screen touch strips exist and reflect physical CC1");
    pitch->setValue(100, juce::sendNotificationSync); pitch->onDragEnd(); mod->setValue(33, juce::sendNotificationSync);
    waveCheck(b.currentDetune() == 0 && b.modulation.load() == 33, "Mouse Pitch springs to centre while mouse Mod latches its value");
    const auto quote = DroneEditor::footerQuote();
    waveCheck(quote == juce::String::fromUTF8(u8"Никто не обольщай самого себя. Если кто из вас думает быть мудрым в веке сем, тот будь безумным, чтобы быть мудрым"), "Cyrillic quotation matches the user's exact text");
    const auto face = editor->getLookAndFeel().getTextButtonFont(*panic, 30).getTypefacePtr();
    bool glyphs = true;
    for (int i = 0; i < quote.length(); ++i) if (quote[i] > 32)
    {
        const bool present = face->getNominalGlyphForCodepoint(quote[i]).has_value();
        glyphs = glyphs && present;
    }
    waveCheck(glyphs, "Embedded ecclesiastical typeface contains every Cyrillic and punctuation glyph in the quote");
    waveCheck(quote[0] == 0x041d && quote[1] == 0x0438 && quote.length() == 114,
        "Cyrillic quotation is decoded as Unicode rather than the Windows source codepage");
    DroneProcessor full; setPlain(full, "reverb", 1); setPlain(full, "delay", 1); setPlain(full, "attack", 0.005f);
    setPlain(full, "eqBass", 12); setPlain(full, "eqMid", 12); setPlain(full, "eqTreble", 12); full.prepareToPlay(48000, 256);
    bool safe = true;
    for (int block = 0; block < 400; ++block)
    {
        juce::MidiBuffer m;
        if (block == 0) for (int n = 48; n < 88; n += 5) m.addEvent(juce::MidiMessage::noteOn(1, n, 1.0f), 0);
        m.addEvent(juce::MidiMessage::pitchWheel(1, block % 3 == 0 ? 0 : block % 3 == 1 ? 8192 : 16383), 64);
        m.addEvent(juce::MidiMessage::controllerEvent(1, 1, block % 128), 70);
        if (block % 7 == 0)
        {
            m.addEvent(juce::MidiMessage::controllerEvent(1, 15, block % 128), 80);
            m.addEvent(juce::MidiMessage::controllerEvent(1, 30, (block * 3) % 128), 90);
            m.addEvent(juce::MidiMessage::controllerEvent(1, 31, (block * 7) % 128), 100);
            m.addEvent(juce::MidiMessage::controllerEvent(1, 102 + block % 8, 127), 110);
        }
        full.processBlock(audio, m);
        for (int ch = 0; ch < 2; ++ch) for (int i = 0; i < 256; ++i) safe = safe && std::isfinite(audio.getSample(ch, i)) && std::abs(audio.getSample(ch, i)) <= 0.940001f;
    }
    waveCheck(safe && full.voices.load() == 8, "Eight voices, pads, Pitch, Mod, automated EQ, full Reverb and Delay remain bounded together");
    full.panicRequested.store(true); float residual = 0;
    for (int block = 0; block < 200; ++block)
    {
        juce::MidiBuffer m; full.processBlock(audio, m);
        if (block > 100) residual = std::max(residual, audio.getMagnitude(0, 256));
    }
    waveCheck(full.voices.load() == 0 && residual < 0.000001f,
        "Panic remains silent after the combined EQ, effects and controller stress test");
}
