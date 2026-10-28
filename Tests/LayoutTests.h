// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "WaveformTests.h"
#include "../Source/PluginEditor.h"

inline void runLayoutTests()
{
    DroneProcessor p;neutralTestSound(p);
    setPlain(p,"attack",.005f);setPlain(p,"release",.25f);
    p.prepareToPlay(48000,256);
    std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
    juce::Slider* poly=nullptr;
    juce::TextButton *pray=nullptr,*silence=nullptr;
    for(auto* child:editor->getChildren())
    {
        if(child->getName()=="Polyphony")poly=dynamic_cast<juce::Slider*>(child);
        if(child->getName()=="Pray for Inspiration")pray=dynamic_cast<juce::TextButton*>(child);
        if(auto* button=dynamic_cast<juce::TextButton*>(child))
            if(button->getButtonText()=="SILENCE")silence=button;
    }
    waveCheck(poly&&pray&&silence&&pray->getBounds()==juce::Rectangle<int>(38,476,186,28)
        &&pray->getBottom()<silence->getY()&&poly->getBottom()<pray->getY(),
        "Left panel stacks Polyphony, Prayer and Silence without overlap");
    auto display=[&]{return poly->getTextFromValue(poly->getValue());};
    waveCheck(display()=="0 / 8","New instance shows zero actual voices out of eight");
    poly->setValue(9,juce::sendNotificationSync);
    waveCheck(display()=="0 / 9"&&p.polyphonyLimit()==9,"Increasing polyphony changes the maximum, not the active count");
    poly->setValue(8,juce::sendNotificationSync);
    juce::AudioBuffer<float> audio(2,256);
    auto send=[&](juce::MidiMessage message)
    {juce::MidiBuffer midi;midi.addEvent(message,0);p.processBlock(audio,midi);};
    for(int note:{60,64,67})send(juce::MidiMessage::noteOn(1,note,.8f));
    waveCheck(display()=="3 / 8","Three sounding synth voices appear in the Polyphony readout");
    for(int note:{60,64,67})send(juce::MidiMessage::noteOff(1,note));
    waveCheck(display()=="3 / 8","Note Off retains all three voices during their Release");
    for(int block=0;block<80;++block){juce::MidiBuffer none;p.processBlock(audio,none);}
    waveCheck(display()=="0 / 8","Completed Release returns the real active voice count to zero");
    setPlain(p,"release",10);for(int note:{60,64,67})send(juce::MidiMessage::noteOn(1,note,.8f));
    poly->setValue(1,juce::sendNotificationSync);
    for(int block=0;block<10;++block){juce::MidiBuffer none;p.processBlock(audio,none);}
    waveCheck(p.voices.load()==1&&display()=="1 / 1","Reducing the limit during playback follows the engine's actual voice allocation");
    poly->setValue(16,juce::sendNotificationSync);
    waveCheck(p.polyphonyLimit()==16,"Polyphony still reaches sixteen without changing its parameter ID");
    silence->onClick();for(int block=0;block<10;++block){juce::MidiBuffer none;p.processBlock(audio,none);}
    waveCheck(display()=="0 / 16","SILENCE clears the engine and the shared active voice readout");
}
