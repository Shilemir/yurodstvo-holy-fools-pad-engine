// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "WaveformTests.h"
#include "../Source/PluginEditor.h"

inline void runInspirationTests(const juce::File& root)
{
    constexpr std::array<const char*,13> ids {"shimmer","mix","attack","release","reverb","delay","harmony","madness","eqBass","eqMid","eqTreble","osc1Waveform","osc2Waveform"};
    DroneProcessor p; p.prepareToPlay(48000,256);setPlain(p,"master",.4321f);setPlain(p,"polyphony",3);
    p.setPitchFromMidi(12000);p.setModulation(99);
    const float master=p.parameters.getRawParameterValue("master")->load(),pitch=p.currentDetune();
    auto before=p.parameters.copyState();
    std::array<int,4> first {},second {};bool changed=true,inRange=true,preserved=true;int matching=0;
    std::array<float,13> previous {};previous.fill(-999);
    for(int iteration=0;iteration<128;++iteration)
    {
        p.randomizeSound();bool difference=false;
        for(size_t i=0;i<ids.size();++i)
        {
            auto* parameter=p.parameters.getParameter(ids[i]);const float value=p.parameters.getRawParameterValue(ids[i])->load();
            inRange=inRange && std::isfinite(value) && value>=parameter->getNormalisableRange().start && value<=parameter->getNormalisableRange().end;
            difference=difference || value!=previous[i];previous[i]=value;
        }
        changed=changed&&difference;
        const int a=p.currentWaveform(0),b=p.currentWaveform(1);++first[static_cast<size_t>(a)];++second[static_cast<size_t>(b)];if(a==b)++matching;
        preserved=preserved && p.parameters.getRawParameterValue("master")->load()==master && p.currentDetune()==pitch
            && p.modulation.load()==99 && p.polyphonyLimit()==3;
    }
    for(auto child:before)
    {
        const auto id=child["id"].toString();bool randomized=false;
        for(const auto* candidate:ids)randomized=randomized || id==candidate;
        if(!randomized && child.hasProperty("id"))preserved=preserved && p.parameters.getRawParameterValue(id)->load()==static_cast<float>(child["value"]);
    }
    waveCheck(changed&&inRange,"128 prayers generate distinct valid configurations through the real parameter state");
    waveCheck(preserved,"Prayers preserve Master, physical Doubt, Faith, polyphony, legacy detune and all MIDI settings");
    waveCheck(*std::min_element(first.begin(),first.end())>0 && *std::min_element(second.begin(),second.end())>0 && matching>0,
        "Both oscillators independently explore all shapes, including matching waveforms");
    juce::MemoryBlock state;p.getStateInformation(state);DroneProcessor restored;
    restored.setStateInformation(state.getData(),static_cast<int>(state.getSize()));bool saved=true;
    for(const auto* id:ids)saved=saved && p.parameters.getRawParameterValue(id)->load()==restored.parameters.getRawParameterValue(id)->load();
    waveCheck(saved&&restored.parameters.getRawParameterValue("master")->load()==master,"Randomized parameters and waveforms round-trip through the saved host state");
    std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());juce::TextButton* pray=nullptr;
    for(auto* c:editor->getChildren())if(c->getName()=="Pray for Inspiration")pray=dynamic_cast<juce::TextButton*>(c);
    waveCheck(pray&&pray->getBounds()==juce::Rectangle<int>(38,476,186,28),"The prayer button sits above SILENCE in the left panel");
    pray->onClick();waveCheck(pray->getToggleState(),"A prayer click gives a short warm visual response");
    bool gui=true;
    for(int index=0;index<8;++index)
    {
        juce::Slider* knob=nullptr;
        for(auto* c:editor->getChildren())if(c->getName()=="Encoder "+juce::String(index+1))knob=dynamic_cast<juce::Slider*>(c);
        gui=gui&&knob&&std::abs(knob->getValue()-p.parameters.getRawParameterValue(ids[static_cast<size_t>(index)])->load())<.002;
    }
    waveCheck(gui,"All eight GUI knobs reflect randomization through parameter attachments");
    bool pads=true,faders=true;
    for(auto* c:editor->getChildren())
    {
        if(c->getName().startsWith("Pad "))
        {
            const int index=c->getName().substring(4).getIntValue()-1;
            if(auto* pad=dynamic_cast<juce::Button*>(c))pads=pads && pad->getToggleState()==(p.currentWaveform(index/4)==index%4);
        }
        if(c->getName().startsWith("Fader "))
        {
            const int index=c->getName().substring(6).getIntValue()-1;
            const char* id=index==0?"eqBass":index==1?"eqMid":index==2?"eqTreble":"master";
            if(auto* f=dynamic_cast<juce::Slider*>(c))faders=faders && std::abs(f->getValue()-p.parameters.getRawParameterValue(id)->load())<.002;
        }
    }
    waveCheck(pads&&faders,"Prayer immediately synchronizes EQ faders, unchanged Master and active waveform pads");

    DroneProcessor pickup;pickup.prepareToPlay(48000,256);pickup.randomizeSound();juce::AudioBuffer<float> audio(2,256);
    auto send=[&](int cc,int value){juce::MidiBuffer m;m.addEvent(juce::MidiMessage::controllerEvent(1,cc,value),0);pickup.processBlock(audio,m);pickup.flushMidiParameters();};
    const float mix=pickup.parameters.getRawParameterValue("mix")->load();const int far=mix>.5f?0:127;
    send(71,far);waveCheck(pickup.parameters.getRawParameterValue("mix")->load()==mix,"First unseen encoder CC establishes a baseline without a jump");
    send(71,juce::jlimit(0,127,far+(far==0?2:-2)));
    waveCheck(std::abs(pickup.parameters.getRawParameterValue("mix")->load()-(mix+(far==0?2:-2)/127.0f))<.001f,
        "Next encoder step changes the randomized value immediately without pickup travel");
    send(17,42);waveCheck(std::abs(pickup.parameters.getRawParameterValue("master")->load()-yuro::maxMasterGain*42.0f/127)<.001f,"F4 Master never enters randomization pickup mode");

    for(double sr:{44100.0,48000.0,96000.0})
    {
        yuro::DroneEngine bell,dry;
        for(auto* e:{&bell,&dry}){e->prepare(sr,true);e->setTargets({0,.5f,30,.5f,1});e->noteOn(1,57,.8f);}
        bell.setSacredTargets(1,0);double early=0,plain=0;std::vector<double> eventTimes;uint64_t events=0;
        for(int i=0;i<static_cast<int>(sr*25);++i)
        {
            const float a=bell.nextSample(),b=dry.nextSample();
            if(i<sr*.4){early+=a*a;plain+=b*b;}
            if(bell.harmonyEvents()!=events){events=bell.harmonyEvents();eventTimes.push_back(i/sr);}
        }
        bool spacing=eventTimes.size()>=3;
        for(size_t i=1;i<eventTimes.size();++i)spacing=spacing && eventTimes[i]-eventTimes[i-1]>=2.99 && eventTimes[i]-eventTimes[i-1]<=9.01;
        waveCheck(early>plain*4,"Harmony onset remains measurable despite a thirty-second oscillator Attack");
        waveCheck(spacing,"Recurring modal events use slow sample-counted three-to-nine second intervals");
        bell.noteOff(1,57);const auto stopped=bell.harmonyEvents();
        for(int i=0;i<static_cast<int>(sr*12);++i)bell.nextSample();
        waveCheck(bell.harmonyEvents()==stopped && bell.activeVoices()==0,"Note Off stops new strikes; existing bell layers fade and retire without hanging voices");
        yuro::DroneEngine sustained;sustained.prepare(sr,true);sustained.setTargets({0,.5f,.005f,.5f,1});sustained.setSacredTargets(1,0);
        sustained.noteOn(1,60,.8f);for(int i=0;i<static_cast<int>(sr*.2);++i)sustained.nextSample();
        sustained.sustain(1,true);sustained.noteOff(1,60);auto prior=sustained.harmonyEvents();
        for(int i=0;i<static_cast<int>(sr*10);++i)sustained.nextSample();
        waveCheck(sustained.harmonyEvents()>prior,"Sustain pedal keeps Harmony scheduling alive until the pedal releases");
        sustained.sustain(1,false);prior=sustained.harmonyEvents();for(int i=0;i<static_cast<int>(sr*8);++i)sustained.nextSample();
        waveCheck(sustained.harmonyEvents()==prior && sustained.activeVoices()==0,"Pedal release stops new strikes and safely releases resonances");
    }
    for(float release:{.5f,5.0f,30.0f})
    {
        yuro::DroneEngine e;e.prepare(48000,true);e.setTargets({0,.5f,.005f,release,1});
        for(int i=0;i<4800;++i)e.nextSample();e.noteOn(1,60,.8f);for(int i=0;i<480;++i)e.nextSample();
        waveCheck(std::abs(e.envelopeLevels()[60]-1)<.001,"Glow reflects the fully opened audio envelope");
        e.noteOff(1,60);for(int i=0;i<static_cast<int>(48000*release*.5f);++i)e.nextSample();
        waveCheck(std::abs(e.envelopeLevels()[60]-.5f)<.015f,"Glow reaches half level halfway through the actual Release");
        for(int i=0;i<static_cast<int>(48000*(release*.5f+.1f));++i)e.nextSample();
        waveCheck(e.envelopeLevels()[60]==0,"Glow extinguishes when the voice envelope finishes");
    }
    yuro::DroneEngine owned;owned.prepare(48000,true);owned.setTargets({0,.5f,.005f,.5f,1});owned.noteOn(1,60,.8f,0);owned.noteOn(1,60,.8f,1);
    for(int i=0;i<4800;++i)owned.nextSample();owned.noteOff(1,60,0);for(int i=0;i<48000;++i)owned.nextSample();
    waveCheck(owned.envelopeLevels()[60]>.99f,"A shared MIDI/mouse pitch stays lit while either owner still holds its voice");
    owned.setPolyphony(1);owned.noteOn(1,64,.8f);const float retiring=owned.envelopeLevels()[60];
    for(int i=0;i<1000;++i)owned.nextSample();
    waveCheck(retiring>0 && owned.envelopeLevels()[60]==0 && owned.envelopeLevels()[64]>0,"Stealing and polyphony reduction contribute fading tails rather than a stuck glow");

    yuro::SacredResonance one,two;one.reset(false);two.reset(false);double repeated=0;
    for(int i=0;i<48000*6;++i)
    {
        if(i==0){one.strike(.9f,.2f,48000);two.strike(.9f,.2f,48000);}
        if(i==48000*4)two.strike(.8f,-.2f,48000);
        const float difference=one.process(0,220,48000,1)-two.process(0,220,48000,1);
        if(i>48000*4)repeated+=difference*difference;
    }
    waveCheck(repeated>.001,"A later shaped strike audibly re-excites the actual resonator bank without resetting its tail");

    DroneProcessor glow;setPlain(glow,"attack",.005f);setPlain(glow,"release",.5f);glow.prepareToPlay(48000,256);
    juce::MidiBuffer on;on.addEvent(juce::MidiMessage::noteOn(1,60,.8f),0);on.addEvent(juce::MidiMessage::noteOn(1,61,.8f),0);glow.processBlock(audio,on);
    for(int i=0;i<100;++i){juce::MidiBuffer m;glow.processBlock(audio,m);}
    PlayableKeyboard keys(glow);keys.setSize(945,80);juce::Image glowSheet(juce::Image::RGB,945,240,true);
    {
        juce::Graphics sheet(glowSheet);sheet.drawImageAt(keys.createComponentSnapshot(keys.getLocalBounds()),0,0);
        waveCheck(glow.noteLevels[60].load()>.99f,"The processor publishes the actual envelope level for GUI consumption");
        juce::MidiBuffer off;off.addEvent(juce::MidiMessage::allNotesOff(1),0);glow.processBlock(audio,off);
        for(int i=0;i<46;++i){juce::MidiBuffer m;glow.processBlock(audio,m);}
        sheet.drawImageAt(keys.createComponentSnapshot(keys.getLocalBounds()),0,80);
        waveCheck(std::abs(glow.noteLevels[60].load()-.5f)<.03f,"Published keyboard glow decays in step with the audio Release");
        for(int i=0;i<70;++i){juce::MidiBuffer m;glow.processBlock(audio,m);}
        sheet.drawImageAt(keys.createComponentSnapshot(keys.getLocalBounds()),0,160);
        waveCheck(glow.noteLevels[60].load()==0,"Published keyboard glow returns to zero after the voice ends");
    }
    auto glowPng=root.getChildFile("Release-glow-states.png").createOutputStream();juce::PNGImageFormat().writeImageToStream(glowSheet,*glowPng);

    DroneProcessor moving;setPlain(moving,"attack",.005f);setPlain(moving,"polyphony",16);setPlain(moving,"master",.65f);moving.prepareToPlay(48000,256);
    bool safe=true;float peak=0;double elapsed=0;
    for(int block=0;block<3000;++block)
    {
        if(block%101==0)moving.randomizeSound();
        juce::MidiBuffer m;if(block==0)for(int n=0;n<16;++n)m.addEvent(juce::MidiMessage::noteOn(1,48+n*2,.8f),0);
        const double start=juce::Time::getMillisecondCounterHiRes();moving.processBlock(audio,m);elapsed+=juce::Time::getMillisecondCounterHiRes()-start;
        for(int ch=0;ch<2;++ch)for(int i=0;i<256;++i){float x=audio.getSample(ch,i);safe=safe&&std::isfinite(x)&&std::abs(x)<=.940001f;peak=std::max(peak,std::abs(x));}
    }
    waveCheck(safe&&moving.parameters.getRawParameterValue("master")->load()==.65f,"Repeated prayers during sixteen held notes preserve Master and safe finite audio");
    std::cout<<"Prayer sixteen-voice stress: "<<elapsed<<" ms / 16000 ms audio, peak="<<peak<<std::endl;
    root.getChildFile("Inspiration-stress.txt").replaceWithText("Audio processing ms: "+juce::String(elapsed,3)+" / 16000 ms audio\nPeak: "+juce::String(peak,6)+"\n");
}
