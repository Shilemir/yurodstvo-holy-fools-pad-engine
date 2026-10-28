// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout DroneProcessor::makeParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout result;
    const std::array<const char*, 5> names { "Oscillator Detune", "Oscillator Mix", "Attack", "Release", "Master Volume" };
    const std::array<const char*, 5> units { "cent", "", "s", "s", "" };
    const std::array<float, 5> defaults { 7, 0.5f, 4, 8, 0.65f };
    std::array<juce::NormalisableRange<float>, 5> ranges {
        juce::NormalisableRange<float>(-50, 50, 0.01f),
        juce::NormalisableRange<float>(0, 1, 0.0001f),
        juce::NormalisableRange<float>(0.005f, 30, 0.001f),
        juce::NormalisableRange<float>(0.005f, 30, 0.001f),
        juce::NormalisableRange<float>(0, 1, 0.0001f) };
    ranges[2].setSkewForCentre(5.0f); ranges[3].setSkewForCentre(5.0f);
    for (size_t i = 0; i < names.size(); ++i)
        result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(yuro::parameterIds[i], 1),
            names[i], ranges[i], defaults[i], juce::AudioParameterFloatAttributes().withLabel(units[i])));
    const juce::StringArray shapes { "Sine", "Triangle", "Sawtooth", "Square" };
    result.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("osc1Waveform", 1), "Oscillator 1 Waveform", shapes, 0));
    result.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("osc2Waveform", 1), "Oscillator 2 Waveform", shapes, 0));
    result.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("padChannel", 1), "Pad MIDI Channel (0 = Any)", 0, 16, 0));
    result.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("padFirstNote", 1), "Pad First Note", 0, 120, 36));
    result.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("padFirstCC", 1), "Pad First CC", 0, 120, 102));
    result.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("padInput", 1), "Pad Input Type", juce::StringArray { "Notes", "CC", "MiniLab Auto", "Learned" }, 2));
    result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverb", 1), "Reverb", juce::NormalisableRange<float>(0, 1, 0.0001f), 0.0f));
    result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("delay", 1), "Delay", juce::NormalisableRange<float>(0, 1, 0.0001f), 0.0f));
    for (const auto& band : { std::pair<const char*, const char*>("eqBass", "Bass"), { "eqMid", "Mid" }, { "eqTreble", "Treble" } })
        result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(band.first, 1), band.second,
            juce::NormalisableRange<float>(-12, 12, 0.01f), 0.0f, juce::AudioParameterFloatAttributes().withLabel("dB")));
    result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("pitchDetune", 1), "Pitch Detune",
        juce::NormalisableRange<float>(-100, 100), 0.0f, juce::AudioParameterFloatAttributes().withLabel("ct")));
    result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("shimmer", 1), "Shimmer",
        juce::NormalisableRange<float>(0, 1, 0.0001f), 0.0f));
    result.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("polyphony", 1), "Polyphony", 1, 16, 8));
    for (const auto& item : { std::pair<const char*,const char*>("harmony","Harmony"), {"madness","Madness"} })
        result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(item.first,1), item.second,
            juce::NormalisableRange<float>(0,1,.0001f),0.0f));
    return result;
}
DroneProcessor::DroneProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "YURODSTVO_STATE", makeParameters())
{
    for (size_t i = 0; i < raw.size(); ++i)
    {
        raw[i] = parameters.getRawParameterValue(yuro::parameterIds[i]);
        parameterObjects[i] = parameters.getParameter(yuro::parameterIds[i]);
        pending[i].store(-1);
    }
    for (auto& n : notes) n.store(false);
    for(auto& level:noteLevels)level.store(0);
    for(auto& armed:pickupArmed)armed.store(false);previousControl.fill(-1);
    for (size_t osc = 0; osc < 2; ++osc)
    {
        waveObjects[osc] = parameters.getParameter(yuro::waveformIds[osc]);
        waveRaw[osc] = parameters.getRawParameterValue(yuro::waveformIds[osc]);
        pendingWaves[osc].store(-1);
    }
    constexpr std::array<const char*, 4> padIds { "padChannel", "padFirstNote", "padFirstCC", "padInput" };
    for (size_t i = 0; i < padIds.size(); ++i) padConfig[i] = parameters.getRawParameterValue(padIds[i]);
    constexpr std::array<const char*, 2> effectIds { "reverb", "delay" };
    for (size_t i = 0; i < effectIds.size(); ++i)
    {
        effectObjects[i] = parameters.getParameter(effectIds[i]);
        effectRaw[i] = parameters.getRawParameterValue(effectIds[i]); pendingEffects[i].store(-1);
    }
    constexpr std::array<const char*, 3> eqIds { "eqBass", "eqMid", "eqTreble" };
    for (size_t i = 0; i < 3; ++i)
    {
        eqObjects[i] = parameters.getParameter(eqIds[i]); eqRaw[i] = parameters.getRawParameterValue(eqIds[i]); pendingEQ[i].store(-1);
    }
    pitchParameter = parameters.getParameter("pitchDetune"); pitchRaw = parameters.getRawParameterValue("pitchDetune");
    shimmerParameter = parameters.getParameter("shimmer"); shimmerRaw = parameters.getRawParameterValue("shimmer");
    polyphonyRaw = parameters.getRawParameterValue("polyphony");
    for(size_t i=0; i<2; ++i)
    {
        const char* id=i==0 ? "harmony" : "madness";
        sacredParameters[i]=parameters.getParameter(id); sacredRaw[i]=parameters.getRawParameterValue(id);
        pendingSacred[i].store(-1);
    }
    parameters.addParameterListener("detune", this); parameters.addParameterListener("pitchDetune", this);
    startTimerHz(60);
}
DroneProcessor::~DroneProcessor()
{
    stopTimer(); parameters.removeParameterListener("detune", this); parameters.removeParameterListener("pitchDetune", this);
}
void DroneProcessor::parameterChanged(const juce::String& id, float)
{
    if (restoringState.load()) return;
    detuneSource.store(id == "detune" ? 0 : 1);
    if (id == "pitchDetune") pitchActive.store(false);
}
float DroneProcessor::currentDetune() const
{
    return pitchActive.load() ? pitchToCents(pitchValue.load()) : pitchRaw->load();
}
void DroneProcessor::setPitchFromMidi(int value)
{
    pitchValue.store(juce::jlimit(0, 16383, value)); pitchActive.store(true);
}
void DroneProcessor::setPitchFromMouse(float cents)
{
    pitchActive.store(false); detuneSource.store(1);
    pitchParameter->beginChangeGesture(); pitchParameter->setValueNotifyingHost(pitchParameter->convertTo0to1(cents)); pitchParameter->endChangeGesture();
}
std::array<float, 3> DroneProcessor::effectiveEQ() const
{
    std::array<float, 3> result {};
    for (size_t i = 0; i < 3; ++i)
    {
        const float value = pendingEQ[i].load(); result[i] = value >= 0 ? eqObjects[i]->convertFrom0to1(value) : eqRaw[i]->load();
    }
    return result;
}
bool DroneProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    return layout.getMainInputChannelSet().isDisabled()
        && (layout.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
            || layout.getMainOutputChannelSet() == juce::AudioChannelSet::mono());
}
std::array<float, yuro::parameterCount> DroneProcessor::effectiveValues() const
{
    std::array<float, yuro::parameterCount> values {};
    for (size_t i = 0; i < values.size(); ++i)
    {
        const auto cc = pending[i].load(std::memory_order_acquire);
        values[i] = cc >= 0 ? parameterObjects[i]->convertFrom0to1(cc) : raw[i]->load();
    }
    values[0] = raw[0]->load(); // Relative oscillator offset is independent of global pitch.
    return values;
}
void DroneProcessor::prepareToPlay(double sr, int)
{
    engine.prepare(sr, true);
    mouseInput.suppress(); audioMouseNote = -1; mouseEpoch.fetch_add(1);
    cancelMouseRequested.store(false); voices.store(0); for (auto& note : notes) note.store(false);
    for(auto& level:noteLevels)level.store(0);
    for(auto& armed:pickupArmed)armed.store(false);previousControl.fill(-1);
    engine.setPolyphony(polyphonyLimit());
    pitchActive.store(false); pitchValue.store(8192); modulation.store(0);
    equalizer.prepare(sr, effectiveEQ());
    engine.setTargets(effectiveValues());
    engine.setPitchCents(currentDetune());
    updateSacredTargets();
    engine.setWaveforms({ currentWaveform(0), currentWaveform(1) });
    effects.prepare(sr, effectiveValues()[4]); updateEffectTargets();
}
void DroneProcessor::updateEffectTargets()
{
    std::array<float, 2> values {};
    for (size_t i = 0; i < 2; ++i)
    {
        const float pendingValue = pendingEffects[i].load();
        values[i] = pendingValue >= 0 ? pendingValue : effectRaw[i]->load();
    }
    effects.setTargets(values[0], values[1], effectiveValues()[4]);
    const float shimmer = pendingShimmer.load(); effects.setShimmer(shimmer >= 0 ? shimmer : shimmerRaw->load());
}
void DroneProcessor::updateSacredTargets()
{
    std::array<float,2> value {};
    for(size_t i=0;i<2;++i)
    { const float pendingValue=pendingSacred[i].load(); value[i]=pendingValue>=0 ? pendingValue : sacredRaw[i]->load(); }
    engine.setSacredTargets(value[0],value[1]);
}
void DroneProcessor::processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    audio.clear();
    updateSacredTargets();
    engine.setPolyphony(polyphonyLimit());
    engine.setTargets(effectiveValues());
    engine.setPitchCents(currentDetune());
    engine.setWaveforms({ currentWaveform(0), currentWaveform(1) });
    updateEffectTargets();
    equalizer.setTargets(effectiveEQ());
    const int padChannel = juce::roundToInt(padConfig[0]->load());
    const int firstNote = juce::roundToInt(padConfig[1]->load());
    const int firstCC = juce::roundToInt(padConfig[2]->load());
    const int padMode = juce::roundToInt(padConfig[3]->load());
    if (cancelMouseRequested.exchange(false)) cancelMouseInputThrough(cancelMouseThrough.load());
    if (panicRequested.exchange(false)) { engine.allNotesOff(0, true); effects.beginPanic(); cancelMouseInput(); }
    mouseInput.drain([&](int note)
    {
        if (note == audioMouseNote) return;
        const int previous = audioMouseNote;
        if (audioMouseNote >= 0) engine.noteOff(1, audioMouseNote, 1);
        audioMouseNote = note;
        if (note >= 0) engine.noteOn(1, note, 0.8f, 1);
        lastMidiType.store(note >= 0 ? 1 : 2); lastMidiChannel.store(1);
        lastNote.store(note >= 0 ? note : previous);
    });
    int cursor = 0;
    float blockPeak = 0;
    auto renderUntil = [&](int end)
    {
        while (cursor < end)
        {
            const auto stereo = engine.nextStereoSample();
            const bool wasPanicFading = effects.isPanicFading();
            const auto sample = effects.process(equalizer.process(stereo));
            // Clear biquad memory at the existing silent FX reset, so its tiny
            // ring-out cannot reseed delay/reverb after PANIC has cleared them.
            if (wasPanicFading && !effects.isPanicFading()) equalizer.reset();
            blockPeak = juce::jmax(blockPeak, std::abs(sample[0]), std::abs(sample[1]));
            if (audio.getNumChannels() == 1) audio.setSample(0, cursor, 0.5f * (sample[0] + sample[1]));
            else for (int ch = 0; ch < audio.getNumChannels(); ++ch) audio.setSample(ch, cursor, sample[static_cast<size_t>(ch % 2)]);
            ++cursor;
        }
    };
    for (const auto event : midi)
    {
        renderUntil(juce::jlimit(cursor, audio.getNumSamples(), event.samplePosition));
        if (event.numBytes > 3) continue; // Ignore SysEx without constructing an allocating MidiMessage.
        const auto message = event.getMessage();
        if (message.isPitchWheel())
        {
            setPitchFromMidi(message.getPitchWheelValue());
            lastMidiType.store(4); lastMidiChannel.store(message.getChannel());
            engine.setTargets(effectiveValues());
            engine.setPitchCents(currentDetune()); continue;
        }
        if (message.isNoteOnOrOff())
        {
            lastNote.store(message.getNoteNumber()); lastMidiChannel.store(message.getChannel());
            lastMidiType.store(message.isNoteOn() ? 1 : 2);
        }
        if (message.isController())
        {
            lastCC.store(message.getControllerNumber()); lastCCValue.store(message.getControllerValue());
            lastMidiChannel.store(message.getChannel()); lastMidiType.store(3);
            const int cc = message.getControllerNumber(), value = message.getControllerValue();
            if (cc == 1) { setModulation(value); continue; } // FAITH is visual only.
            const int control = yuro::controlIndex(cc);
            if (control >= 0)
            {
                if(waitForPickup(control,value))continue;
                if (control >= 10)
                {
                    pendingSacred[static_cast<size_t>(control-10)].store(static_cast<float>(value)/127);
                    lastControlValue.store(value); lastControlChannel.store(message.getChannel()); lastControlCC.store(cc);
                    lastControl.store(control); controlHits.fetch_add(1); updateSacredTargets(); continue;
                }
                if (control == 0)
                {
                    pendingShimmer.store(static_cast<float>(value) / 127);
                    lastControlValue.store(value); lastControlChannel.store(message.getChannel()); lastControlCC.store(cc);
                    lastControl.store(0); controlHits.fetch_add(1); updateEffectTargets(); continue;
                }
                // Handle parameter CCs BEFORE pad routing, regardless of pad mode/channel.
                if (control < 4) pending[static_cast<size_t>(control)].store(static_cast<float>(value) / 127);
                else if (control < 6) pendingEffects[static_cast<size_t>(control - 4)].store(static_cast<float>(value) / 127);
                else if (control == 6) pending[4].store(static_cast<float>(value) / 127);
                else pendingEQ[static_cast<size_t>(control - 7)].store(eqObjects[static_cast<size_t>(control - 7)]->convertTo0to1(ThreeBandEQ::ccToDb(value)));
                lastControlValue.store(value); lastControlChannel.store(message.getChannel());
                lastControlCC.store(cc);
                lastControl.store(control); controlHits.fetch_add(1);
                engine.setTargets(effectiveValues());
                engine.setPitchCents(currentDetune()); updateEffectTargets(); equalizer.setTargets(effectiveEQ());
                continue;
            }
            if (cc == 64 || cc == 120 || cc == 123)
            {
                if (cc == 64) engine.sustain(message.getChannel(), value >= 64);
                if (cc == 123) engine.allNotesOff(message.getChannel(), false);
                if (cc == 120) { engine.allNotesOff(message.getChannel(), true); effects.beginPanic(); cancelMouseInput(); }
                continue;
            }
        }
        const int requestedMode = pendingPadMode.load();
        const int mappedPad = padRouter.handle(message, requestedMode >= 0 ? requestedMode : padMode, padChannel, firstNote, firstCC);
        if (padRouter.completed.exchange(false)) pendingPadMode.store(3);
        if (mappedPad != -1)
        {
            if (mappedPad >= 0)
            {
                pendingWaves[static_cast<size_t>(mappedPad / 4)].store(mappedPad % 4);
                engine.setWaveforms({ currentWaveform(0), currentWaveform(1) });
            }
            if (message.isNoteOff()) engine.noteOff(message.getChannel(), message.getNoteNumber());
            continue;
        }
        if (message.isNoteOn()) engine.noteOn(message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isNoteOff()) engine.noteOff(message.getChannel(), message.getNoteNumber());
    }
    renderUntil(audio.getNumSamples());
    midi.clear();
    peak.store(blockPeak); voices.store(engine.activeVoices());
    for (size_t n = 0; n < notes.size(); ++n) notes[n].store(engine.sounding(static_cast<int>(n)));
    const auto levels=engine.envelopeLevels();
    for(size_t n=0;n<levels.size();++n)noteLevels[n].store(levels[n],std::memory_order_relaxed);
}
juce::RangedAudioParameter* DroneProcessor::controlParameter(int control) const
{
    if(control==0)return shimmerParameter;
    if(control<4)return parameterObjects[static_cast<size_t>(control)];
    if(control<6)return effectObjects[static_cast<size_t>(control-4)];
    if(control==6)return parameterObjects[4];
    if(control<10)return eqObjects[static_cast<size_t>(control-7)];
    return sacredParameters[static_cast<size_t>(control-10)];
}
bool DroneProcessor::waitForPickup(int control,int value)
{
    const size_t index=static_cast<size_t>(control);
    const int previous=previousControl[index];previousControl[index]=value;
    if(!pickupArmed[index].load())return false;
    const float target=controlParameter(control)->getValue()*127;
    const bool near=std::abs(static_cast<float>(value)-target)<=1.5f;
    const bool crossed=previous>=0 && (previous-target)*(value-target)<=0;
    if(near || crossed){pickupArmed[index].store(false);return false;}
    return true;
}
void DroneProcessor::randomizeSound()
{
    // Only these controls use pickup. Master and expression are never touched.
    for(size_t i=0;i<pickupArmed.size();++i)if(i!=6)pickupArmed[i].store(true);
    pendingShimmer.store(-1);for(size_t i=1;i<4;++i)pending[i].store(-1);
    for(auto& v:pendingEffects)v.store(-1);for(auto& v:pendingEQ)v.store(-1);
    for(auto& v:pendingSacred)v.store(-1);for(auto& v:pendingWaves)v.store(-1);
    constexpr std::array<const char*,13> ids {"shimmer","mix","attack","release","reverb","delay","harmony","madness","eqBass","eqMid","eqTreble","osc1Waveform","osc2Waveform"};
    for(size_t i=0;i<ids.size();++i)
    {
        auto* parameter=parameters.getParameter(ids[i]);
        float value=inspirationRandom.nextFloat();
        if(i==2 || i==3)
        {
            if(i==3)value=std::sqrt(value);
            const float seconds=inspirationRandom.nextFloat()<.2f ? .005f+29.995f*value
                : std::exp(std::log(.005f)+value*std::log(6000.0f));
            value=parameter->convertTo0to1(seconds);
        }
        else if(i>=8 && i<11)
        {
            // Mostly centred EQ, with occasional full-range extremes.
            if(inspirationRandom.nextFloat()<.8f)value=.5f*(value+inspirationRandom.nextFloat());
        }
        else if(i>=11)value=parameter->convertTo0to1(static_cast<float>(inspirationRandom.nextInt(4)));
        parameter->beginChangeGesture();parameter->setValueNotifyingHost(value);parameter->endChangeGesture();
    }
}
void DroneProcessor::flushMidiParameters()
{
    for(size_t i=0;i<2;++i)
    {
        auto value=pendingSacred[i].load(); if(value<0) continue;
        auto* p=sacredParameters[i]; p->beginChangeGesture(); p->setValueNotifyingHost(value); p->endChangeGesture();
        pendingSacred[i].compare_exchange_strong(value,-1.0f);
    }
    auto shimmer = pendingShimmer.load();
    if (shimmer >= 0)
    {
        shimmerParameter->beginChangeGesture(); shimmerParameter->setValueNotifyingHost(shimmer); shimmerParameter->endChangeGesture();
        pendingShimmer.compare_exchange_strong(shimmer, -1.0f);
    }
    for (size_t i = 0; i < 3; ++i)
    {
        auto value = pendingEQ[i].load(); if (value < 0) continue;
        auto* p = eqObjects[i]; p->beginChangeGesture(); p->setValueNotifyingHost(value); p->endChangeGesture();
        pendingEQ[i].compare_exchange_strong(value, -1.0f);
    }
    auto requestedMode = pendingPadMode.load();
    if (requestedMode >= 0)
    {
        auto* p = parameters.getParameter("padInput");
        p->beginChangeGesture(); p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(requestedMode))); p->endChangeGesture();
        pendingPadMode.compare_exchange_strong(requestedMode, -1);
    }
    for (size_t i = 0; i < 2; ++i)
    {
        auto value = pendingEffects[i].load();
        if (value < 0) continue;
        auto* p = effectObjects[i];
        p->beginChangeGesture(); p->setValueNotifyingHost(value); p->endChangeGesture();
        pendingEffects[i].compare_exchange_strong(value, -1.0f);
    }
    for (size_t i = 0; i < pending.size(); ++i)
    {
        auto value = pending[i].load(std::memory_order_acquire);
        if (value < 0) continue;
        auto* p = parameterObjects[i];
        p->beginChangeGesture(); p->setValueNotifyingHost(value); p->endChangeGesture();
        pending[i].compare_exchange_strong(value, -1.0f, std::memory_order_acq_rel);
    }
    for (size_t osc = 0; osc < 2; ++osc)
    {
        auto value = pendingWaves[osc].load();
        if (value < 0) continue;
        auto* p = waveObjects[osc];
        p->beginChangeGesture(); p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(value))); p->endChangeGesture();
        pendingWaves[osc].compare_exchange_strong(value, -1);
    }
}
void DroneProcessor::cancelMouseInput()
{
    cancelMouseInputThrough(mouseInput.watermark()); mouseEpoch.fetch_add(1);
}
void DroneProcessor::cancelMouseInputThrough(uint32_t cutoff)
{
    if (audioMouseNote >= 0) engine.noteOff(1, audioMouseNote, 1);
    audioMouseNote = -1; mouseInput.suppressThrough(cutoff);
}
void DroneProcessor::useAutoPads()
{
    padRouter.cancelLearning(); pendingPadMode.store(-1);
    constexpr std::array<const char*, 4> ids { "padInput", "padChannel", "padFirstNote", "padFirstCC" };
    constexpr std::array<float, 4> values { 2, 0, 36, 102 };
    for (size_t i = 0; i < ids.size(); ++i)
    {
        auto* p = parameters.getParameter(ids[i]);
        p->beginChangeGesture(); p->setValueNotifyingHost(p->convertTo0to1(values[i])); p->endChangeGesture();
    }
}
int DroneProcessor::currentWaveform(int oscillator) const
{
    const auto index = static_cast<size_t>(juce::jlimit(0, 1, oscillator));
    const int pendingValue = pendingWaves[index].load();
    return juce::jlimit(0, 3, pendingValue >= 0 ? pendingValue : juce::roundToInt(waveRaw[index]->load()));
}
void DroneProcessor::selectWaveform(int oscillator, int shape)
{
    const auto index = static_cast<size_t>(juce::jlimit(0, 1, oscillator));
    pendingWaves[index].store(-1);
    auto* p = waveObjects[index];
    p->beginChangeGesture();
    p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(juce::jlimit(0, 3, shape))));
    p->endChangeGesture();
}
void DroneProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    // copyState is deliberately outside processBlock. Include the latest CC even if
    // the message-thread timer has not run yet. Never call host callbacks here.
    auto state = parameters.copyState();
    state.setProperty("schemaVersion", 8, nullptr);
    for(size_t i=0;i<2;++i)
        if(const float value=pendingSacred[i].load(); value>=0)
            state.getChildWithProperty("id",i==0 ? "harmony" : "madness").setProperty("value",value,nullptr);
    if (const float shimmer = pendingShimmer.load(); shimmer >= 0)
        state.getChildWithProperty("id", "shimmer").setProperty("value", shimmer, nullptr);
    state.setProperty("detuneSource", detuneSource.load(), nullptr);
    for (size_t i = 0; i < 3; ++i)
        if (const float value = pendingEQ[i].load(); value >= 0)
            state.getChildWithProperty("id", i == 0 ? "eqBass" : i == 1 ? "eqMid" : "eqTreble").setProperty("value", eqObjects[i]->convertFrom0to1(value), nullptr);
    for (size_t i = 0; i < 2; ++i)
        if (const float value = pendingEffects[i].load(); value >= 0)
            state.getChildWithProperty("id", i == 0 ? "reverb" : "delay").setProperty("value", value, nullptr);
    if (const int mode = pendingPadMode.load(); mode >= 0)
        state.getChildWithProperty("id", "padInput").setProperty("value", mode, nullptr);
    if (auto oldMap = state.getChildWithName("PAD_LEARN"); oldMap.isValid()) state.removeChild(oldMap, nullptr);
    juce::ValueTree learned("PAD_LEARN");
    for (size_t i = 0; i < 8; ++i) learned.setProperty("p" + juce::String(static_cast<int>(i)), padRouter.learned[i].load(), nullptr);
    state.appendChild(learned, nullptr);
    for (size_t i = 0; i < pending.size(); ++i)
    {
        const auto cc = pending[i].load();
        if (cc >= 0)
            for (auto child : state)
                if (child["id"].toString() == yuro::parameterIds[i])
                    child.setProperty("value", parameterObjects[i]->convertFrom0to1(cc), nullptr);
    }
    for (size_t osc = 0; osc < 2; ++osc)
        if (const int shape = pendingWaves[osc].load(); shape >= 0)
            for (auto child : state)
                if (child["id"].toString() == yuro::waveformIds[osc]) child.setProperty("value", shape, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary(*xml, dest);
}
void DroneProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(parameters.state.getType()))
        {
            for (auto& p : pending) p.store(-1);
            for (auto& p : pendingWaves) p.store(-1);
            for (auto& p : pendingEffects) p.store(-1);
            for (auto& p : pendingEQ) p.store(-1);
            cancelMouseThrough.store(mouseInput.watermark()); cancelMouseRequested.store(true); mouseEpoch.fetch_add(1);
            pendingShimmer.store(-1);
            for(auto& armed:pickupArmed)armed.store(false);
            for(auto& value:pendingSacred) value.store(-1);
            pitchActive.store(false); pitchValue.store(8192); modulation.store(0);
            pendingPadMode.store(-1); padRouter.cancelLearning();
            auto state = juce::ValueTree::fromXml(*xml);
            // Old 0.1/0.2 states have five parameters. Missing new parameters
            // migrate to Sine + Sine and the default MiniLab mapping, not the
            // previous state of whichever instance is being reused.
            constexpr std::array<const char*, 16> newIds {
                "osc1Waveform", "osc2Waveform", "padChannel", "padFirstNote", "padFirstCC", "padInput", "reverb", "delay", "eqBass", "eqMid", "eqTreble", "pitchDetune", "shimmer", "polyphony", "harmony", "madness" };
            constexpr std::array<float, 16> defaults { 0, 0, 0, 36, 102, 2, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0 };
            for (size_t i = 0; i < newIds.size(); ++i)
                if (!state.getChildWithProperty("id", newIds[i]).isValid())
                {
                    juce::ValueTree child("PARAM"); child.setProperty("id", newIds[i], nullptr);
                    child.setProperty("value", defaults[i], nullptr); state.appendChild(child, nullptr);
                }
            if (static_cast<int>(state.getProperty("schemaVersion", 0)) < 4
                && static_cast<int>(state.getChildWithProperty("id", "padInput")["value"]) == 0
                && static_cast<int>(state.getChildWithProperty("id", "padChannel")["value"]) == 10
                && static_cast<int>(state.getChildWithProperty("id", "padFirstNote")["value"]) == 36
                && static_cast<int>(state.getChildWithProperty("id", "padFirstCC")["value"]) == 102)
            {
                state.getChildWithProperty("id", "padInput").setProperty("value", 2, nullptr);
                state.getChildWithProperty("id", "padChannel").setProperty("value", 0, nullptr);
            }
            const auto learned = state.getChildWithName("PAD_LEARN");
            for (size_t i = 0; i < 8; ++i)
            {
                const int value = learned.isValid() ? static_cast<int>(learned.getProperty("p" + juce::String(static_cast<int>(i)), -1)) : -1;
                padRouter.learned[i].store(value >= 0 && value <= PadRouter::encode(2, 16, 127) ? value : -1);
            }
            const int source = static_cast<int>(state.getProperty("schemaVersion", 0)) < 6 ? 0 : juce::jlimit(0, 1, static_cast<int>(state.getProperty("detuneSource", 1)));
            restoringState.store(true); parameters.replaceState(state); restoringState.store(false); detuneSource.store(source);
        }
}
juce::AudioProcessorEditor* DroneProcessor::createEditor() { return new DroneEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DroneProcessor(); }
