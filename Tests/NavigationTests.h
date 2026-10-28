// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "WaveformTests.h"
#include "../Source/PluginEditor.h"

inline void runNavigationTests()
{
    DroneProcessor p;
    const auto range=[](DroneProcessor& processor,const char* id)
    { return processor.parameters.getParameter(id)->getNormalisableRange(); };
    waveCheck(range(p,"attack").end==20 && range(p,"release").end==20
        && std::abs(range(p,"master").end-yuro::maxMasterGain)<.0001f
        && range(p,"eqBass").start==-15 && range(p,"eqMid").end==15 && range(p,"eqTreble").end==15,
        "Envelope, Master and three EQ parameter ranges match the new limits");
    AmbientEffects reference,boosted;
    reference.prepare(48000,1.0f);boosted.prepare(48000,yuro::maxMasterGain);
    reference.setTargets(0,0,1.0f);boosted.setTargets(0,0,yuro::maxMasterGain);
    const auto plain=reference.process(0.1f)[0],loud=boosted.process(0.1f)[0];
    waveCheck(std::abs(20*std::log10(loud/plain)-3.0f)<.02f,
        "Expanded Master maximum adds about three decibels below the output safety knee");
    waveCheck(p.reservedOctaveMask()==(1u<<3) && !p.playableView(36)
        && p.playableView(48) && p.playableView(60) && !p.playableView(24),
        "The default pad octave 36-47 is reserved in every two-octave keyboard view");
    std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
    PlayableKeyboard* keys=nullptr;juce::TextButton *down=nullptr,*up=nullptr;
    for(auto* child:editor->getChildren())
    {
        if(child->getName()=="On-screen keyboard")keys=dynamic_cast<PlayableKeyboard*>(child);
        if(child->getName()=="Octave Down")down=dynamic_cast<juce::TextButton*>(child);
        if(child->getName()=="Octave Up")up=dynamic_cast<juce::TextButton*>(child);
    }
    waveCheck(keys&&down&&up&&keys->firstNote()==48&&!down->isEnabled()&&up->isEnabled(),
        "MIDI 48 is the default C2; the left arrow excludes pad octave 36-47");
    p.keyboardFirstNote.store(60);
    juce::MemoryBlock viewState;p.getStateInformation(viewState);
    DroneProcessor viewRestored;viewRestored.setStateInformation(viewState.getData(),static_cast<int>(viewState.getSize()));
    waveCheck(viewRestored.keyboardFirstNote.load()==60,
        "An existing saved C3 keyboard view remains unchanged on reopening");
    p.keyboardFirstNote.store(48);
    up->onClick();waveCheck(keys->firstNote()==60&&down->isEnabled(),
        "Octave up updates the playable MIDI range");
    down->onClick();waveCheck(keys->firstNote()==48&&!down->isEnabled(),
        "Octave down restores C2 and stops before the pad octave");
    p.prepareToPlay(48000,256);juce::AudioBuffer<float> audio(2,256);
    auto send=[&](juce::MidiMessage message){juce::MidiBuffer midi;midi.addEvent(message,0);p.processBlock(audio,midi);};
    for(int note=36;note<48;++note)send(juce::MidiMessage::noteOn(1,note,1.0f));
    waveCheck(p.voices.load()==0,"DAW and physical MIDI notes throughout the control octave produce no voices");
    send(juce::MidiMessage::noteOn(1,38,1.0f));
    waveCheck(p.currentWaveform(0)==2&&p.voices.load()==0,"Pad notes still select the corresponding waveform");
    send(juce::MidiMessage::controllerEvent(1,109,127));
    waveCheck(p.currentWaveform(1)==3,"Pad CC selection remains independent of the note octave gate");
    send(juce::MidiMessage::noteOn(1,60,1.0f));
    waveCheck(p.voices.load()==1,"A playable octave still produces an ordinary voice");
    juce::MidiBuffer empty;
    keys->pressAt(PlayableKeyboard::keyBounds(60,48).getCentre());p.processBlock(audio,empty);
    // The view change releases mouse ownership while the physical note remains held.
    up->onClick();p.processBlock(audio,empty);
    waveCheck(keys->firstNote()==60&&p.notes[60].load()&&p.voices.load()>=1,
        "Changing the keyboard view cannot cut an independently held MIDI note");
    send(juce::MidiMessage::noteOff(1,60));

    setPlain(p,"attack",20);setPlain(p,"release",20);setPlain(p,"eqBass",12);setPlain(p,"master",1);
    juce::MemoryBlock saved;p.getStateInformation(saved);
    auto xml=DroneProcessor::getXmlFromBinary(saved.getData(),static_cast<int>(saved.getSize()));
    auto old=juce::ValueTree::fromXml(*xml);
    old.getChildWithProperty("id","attack").setProperty("value",30,nullptr);
    old.getChildWithProperty("id","release").setProperty("value",27,nullptr);
    old.setProperty("keyboardFirstNote",36,nullptr);
    auto oldXml=old.createXml();juce::MemoryBlock legacy;
    DroneProcessor::copyXmlToBinary(*oldXml,legacy);
    DroneProcessor restored;restored.setStateInformation(legacy.getData(),static_cast<int>(legacy.getSize()));
    waveCheck(restored.parameters.getRawParameterValue("attack")->load()==20
        && restored.parameters.getRawParameterValue("release")->load()==20,
        "Legacy envelope values above twenty seconds clamp on state load");
    waveCheck(restored.parameters.getRawParameterValue("eqBass")->load()==12
        && restored.parameters.getRawParameterValue("master")->load()==1
        && restored.playableView(restored.keyboardFirstNote.load()),
        "Plain saved EQ/Master values survive migration and an unsafe keyboard view is corrected");
}
