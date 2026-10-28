// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "WaveformTests.h"
#include "../Source/PluginEditor.h"

inline void compareSacredBypass(const juce::File& root,const juce::String& currentPath)
{
    const auto oldPath=root.getChildFile("0.8.1/VST3/YURODSTVO - Holy Fool's Pad Engine.vst3");
    if(!oldPath.exists()) {std::cout<<"SKIP: optional private 0.8.1 comparison binary unavailable\n";return;}
    juce::VST3PluginFormat format;
    auto load=[&](const juce::String& path)
    {
        juce::OwnedArray<juce::PluginDescription> descriptions;format.findAllTypesForFile(descriptions,path);
        if(descriptions.isEmpty())throw std::runtime_error("Unable to scan compatibility binary");
        juce::String error;auto result=format.createInstanceFromDescription(*descriptions[0],48000,256,error);
        if(!result)throw std::runtime_error(error.toStdString());return result;
    };
    auto before=load(oldPath.getFullPathName()),after=load(currentPath);
    bool ids=true;
    for(int i=0;i<before->getParameters().size();++i)
    {
        auto* old=dynamic_cast<juce::HostedAudioProcessorParameter*>(before->getParameters()[i]);
        bool found=false;
        for(auto* p:after->getParameters())
            if(auto* now=dynamic_cast<juce::HostedAudioProcessorParameter*>(p))
                if(old && old->getParameterID()==now->getParameterID()) found=old->getName(80)==now->getName(80);
        ids=ids&&found;
    }
    waveCheck(ids,"All existing VST3 parameter IDs and host names are unchanged from actual 0.8.1 binary");
    for(auto* p:before->getParameters())
    {
        const auto name=p->getName(80);
        if(name=="Attack")p->setValueNotifyingHost(0);
        if(name=="Reverb" || name=="Delay" || name=="Shimmer")p->setValueNotifyingHost(.35f);
    }
    juce::MemoryBlock state;before->getStateInformation(state);
    after->setStateInformation(state.getData(),static_cast<int>(state.getSize()));
    before->prepareToPlay(48000,256);after->prepareToPlay(48000,256);
    double maxError=0;juce::AudioBuffer<float> a(2,256),b(2,256);
    for(int block=0;block<1200;++block)
    {
        juce::MidiBuffer m;
        if(block==0)for(int n:{48,55,60,64})m.addEvent(juce::MidiMessage::noteOn(1,n,.8f),0);
        if(block==300)m.addEvent(juce::MidiMessage::pitchWheel(1,16383),25);
        if(block==400)m.addEvent(juce::MidiMessage::pitchWheel(1,8192),25);
        if(block==600)m.addEvent(juce::MidiMessage::allNotesOff(1),0);
        auto copy=m;before->processBlock(a,m);after->processBlock(b,copy);
        for(int ch=0;ch<2;++ch)for(int i=0;i<256;++i)maxError=std::max(maxError,static_cast<double>(std::abs(a.getSample(ch,i)-b.getSample(ch,i))));
    }
    before->releaseResources();after->releaseResources();
    std::cout<<"0.8.1 vs 0.9.0 zero-new-modules VST3 maximum error="<<maxError<<std::endl;
    waveCheck(maxError<1e-6,"Old VST3 state renders equivalently with new modules disabled, including FX, pitch and Release");
    bool received=false;
    after->prepareToPlay(48000,256);juce::MidiBuffer m;
    m.addEvent(juce::MidiMessage::controllerEvent(1,116,127),0);m.addEvent(juce::MidiMessage::controllerEvent(1,117,127),0);
    after->processBlock(b,m);after->getStateInformation(state);
    auto reloaded=load(currentPath);reloaded->setStateInformation(state.getData(),static_cast<int>(state.getSize()));
    int matches=0;
    for(auto* p:reloaded->getParameters())if(p->getName(80)=="Harmony" || p->getName(80)=="Madness")
    { received=p->getValue()==1 && p->isAutomatable(); if(received)++matches; }
    waveCheck(matches==2,"Real VST3 forwards E7/E8 CC and restores both new automatable parameters");
    after->releaseResources();
}

inline void runSacredTests(const juce::File& root)
{
    for(double sr : {44100.0,48000.0,96000.0})
    {
        yuro::DroneEngine stable, bypass;
        for(auto* e : {&stable,&bypass})
        { e->prepare(sr); e->setTargets({0,.5f,.005f,1,1}); e->noteOn(1,60,.8f); }
        bypass.setSacredTargets(0,0);
        bool same=true;
        for(int i=0;i<static_cast<int>(sr*2);++i)
        { const auto a=stable.nextStereoSample(),b=bypass.nextStereoSample(); same=same && a==b; }
        waveCheck(same,"Harmony/Madness zero is sample-exact bypass, even with full Hesitation");

        // Measure true modal frequencies on two notes, not just added brightness.
        for(double base : {110.0,220.0})
        {
            yuro::SacredResonance resonance;
            std::vector<float> response(static_cast<size_t>(sr));
            for(auto& x:response) x=resonance.process(0,base,sr,1);
            auto energyAt=[&](double hz)
            {
                double real=0,imaginary=0;
                for(size_t i=0;i<response.size();++i)
                {
                    const double angle=i*hz*juce::MathConstants<double>::twoPi/sr;
                    real+=response[i]*std::cos(angle); imaginary+=response[i]*std::sin(angle);
                }
                return real*real+imaginary*imaginary;
            };
            bool tuned=true;
            for(double ratio : {2.01,2.756,4.07})
                tuned=tuned && energyAt(base*ratio)>energyAt(base*ratio+17)*25;
            waveCheck(tuned,"Harmonic and inharmonic modal peaks track the played note across sample rates");
        }
        yuro::OrganicDrift a,b; a.reset(123,sr); b.reset(456,sr);
        bool continuous=true; float previous=a.next(sr),lo=previous,hi=previous; double independence=0;
        for(int i=0;i<static_cast<int>(sr*15);++i)
        {
            const float x=a.next(sr),y=b.next(sr);
            continuous=continuous && std::abs(x)<=1 && std::abs(x-previous)<.0001f;
            lo=std::min(lo,x);hi=std::max(hi,x);independence+=std::abs(x-y); previous=x;
        }
        waveCheck(continuous && hi-lo>.2f && independence>sr,"Organic modulation varies slowly, continuously and independently on a held note");
    }

    DroneProcessor p; p.prepareToPlay(48000,256); juce::AudioBuffer<float> audio(2,256);
    auto send=[&](int cc,int value) {juce::MidiBuffer m;m.addEvent(juce::MidiMessage::controllerEvent(1,cc,value),37);p.processBlock(audio,m);};
    for(int mode=0;mode<4;++mode)
    {
        setPlain(p,"padInput",static_cast<float>(mode));
        for(const auto& family : {yuro::controlCC,yuro::dawControlCC}) for(int control=10;control<12;++control)
        {
            p.padRouter.learned[0].store(PadRouter::encode(2,1,family[static_cast<size_t>(control)]));
            send(family[static_cast<size_t>(control)],73);
            // Save before message-thread flush: pending control values must survive.
            juce::MemoryBlock state; p.getStateInformation(state); DroneProcessor restored;
            restored.setStateInformation(state.getData(),static_cast<int>(state.getSize()));
            const char* id=control==10?"harmony":"madness";
            waveCheck(std::abs(restored.parameters.getRawParameterValue(id)->load()-73.0f/127)<.001f,
                "New E7/E8 CCs override pad maps and save correctly before host notification");
        }
    }
    p.flushMidiParameters(); send(1,127);
    waveCheck(p.modulation.load()==127 && std::abs(p.parameters.getRawParameterValue("madness")->load()-73.0f/127)<.001f,
        "Hesitation uses CC1 without rewriting saved Madness");
    juce::MemoryBlock state; p.getStateInformation(state); DroneProcessor restored;
    restored.setStateInformation(state.getData(),static_cast<int>(state.getSize()));
    waveCheck(restored.modulation.load()==0 && restored.parameters.getParameter("harmony")->isAutomatable()
        && restored.parameters.getParameter("madness")->isAutomatable(),"New parameters are automatable; transient Hesitation is excluded from saved state");
    auto legacy=p.parameters.copyState();legacy.removeChild(legacy.getChildWithProperty("id","harmony"),nullptr);
    legacy.removeChild(legacy.getChildWithProperty("id","madness"),nullptr);legacy.setProperty("schemaVersion",7,nullptr);
    auto xml=legacy.createXml();juce::MemoryBlock data;juce::AudioProcessor::copyXmlToBinary(*xml,data);
    restored.setStateInformation(data.getData(),static_cast<int>(data.getSize()));
    waveCheck(restored.parameters.getRawParameterValue("harmony")->load()==0 && restored.parameters.getRawParameterValue("madness")->load()==0,
        "Loading old state into a reused instance resets both new modules to zero");

    std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
    for(int encoder=7;encoder<=8;++encoder)
    {
        juce::Slider* knob=nullptr;
        for(auto* child:editor->getChildren()) if(child->getName()=="Encoder "+juce::String(encoder)) knob=dynamic_cast<juce::Slider*>(child);
        waveCheck(knob && knob->isEnabled(),"E7/E8 are enabled, attached GUI controls");
        knob->setValue(.4,juce::sendNotificationSync);
        waveCheck(std::abs(p.parameters.getRawParameterValue(encoder==7?"harmony":"madness")->load()-.4f)<.001f,
            "Mouse edits Harmony/Madness through their saved host parameters");
    }

    // One held note must evolve without any retrigger or controller motion.
    auto renderSingle=[](float madness)
    {
        yuro::DroneEngine engine;engine.prepare(48000,true);engine.setTargets({0,.5f,.005f,1,1});
        engine.setSacredTargets(0,madness);engine.noteOn(1,57,.8f);
        std::array<double,3> result {}; float last=0;
        for(int i=0;i<48000*10;++i)
        {
            const auto y=engine.nextStereoSample();result[0]+=(y[0]-y[1])*(y[0]-y[1]);
            if(i>48000) result[1]=std::max(result[1],static_cast<double>(std::abs(y[0]-last)));
            result[2]+=std::abs(y[0]);last=y[0];
        }
        return result;
    };
    const auto plain=renderSingle(0),base=renderSingle(.4f),deep=renderSingle(.4f);
    waveCheck(plain[0]==0 && base[0]>.01 && deep==base && deep[1]<.02,
        "Madness animates a held note consistently at fixed maximum expression depth");

    auto upperMode=[](float harmony)
    {
        yuro::DroneEngine engine;engine.prepare(48000,true);engine.setTargets({0,.5f,.005f,2,1});
        engine.setSacredTargets(harmony,0);engine.noteOn(1,57,.8f);
        double real=0,imaginary=0;
        for(int i=0;i<96000;++i)
        {
            const float x=engine.nextSample();
            if(i>=4800)
            {const double angle=i*(220*2.756)*juce::MathConstants<double>::twoPi/48000;real+=x*std::cos(angle);imaginary+=x*std::sin(angle);}
        }
        return real*real+imaginary*imaginary;
    };
    waveCheck(upperMode(1)>upperMode(0)*25,"Harmony adds measurable inharmonic bell energy to the actual sine-voice output");
    auto heldPitch=[](float cents)
    {
        yuro::DroneEngine engine;engine.prepare(48000,true);engine.setTargets({0,0,.005f,1,1});
        engine.setSacredTargets(0,1);engine.setPitchCents(cents);engine.noteOn(1,69,.8f);
        int count=0;float previous=0;
        for(int i=0;i<48000*5;++i)
        {const float x=engine.nextSample();if(i>=48000*4 && previous<=0 && x>0)++count;previous=x;}
        return count;
    };
    const double bendCents=1200*std::log2(static_cast<double>(heldPitch(100))/heldPitch(0));
    waveCheck(std::abs(bendCents-100)<8,"Doubt retains its full global semitone bend even with maximum Madness and Hesitation");

    DroneProcessor stress;setPlain(stress,"attack",.005f);setPlain(stress,"release",2);setPlain(stress,"polyphony",16);
    for(const char* id : {"harmony","madness","shimmer","reverb","delay","master"}) setPlain(stress,id,1);
    for(const char* id : {"eqBass","eqMid","eqTreble"}) setPlain(stress,id,12);
    stress.prepareToPlay(48000,256);double time=0,worst=0;bool safe=true;float peak=0;
    for(int block=0;block<4800;++block)
    {
        juce::MidiBuffer m;
        if(block==0) for(int n=0;n<16;++n)m.addEvent(juce::MidiMessage::noteOn(1,48+n*2,1.0f),0);
        if(block%17==0)
        {
            m.addEvent(juce::MidiMessage::controllerEvent(1,1,block%128),13);
            m.addEvent(juce::MidiMessage::pitchWheel(1,block%2?0:16383),38);
            m.addEvent(juce::MidiMessage::controllerEvent(1,102+block%8,127),80);
        }
        const double start=juce::Time::getMillisecondCounterHiRes();stress.processBlock(audio,m);
        const double duration=juce::Time::getMillisecondCounterHiRes()-start;time+=duration;worst=std::max(worst,duration);
        for(int ch=0;ch<2;++ch)for(int i=0;i<256;++i)
        {const float x=audio.getSample(ch,i);safe=safe && std::isfinite(x) && std::abs(x)<=.940001f;peak=std::max(peak,std::abs(x));}
    }
    std::cout<<"Sacred 16-voice maximum stress: CPU time="<<time<<" ms / 25600 ms audio, "<<100*time/25600
        <<"%, worst block="<<worst<<" ms, peak="<<peak<<std::endl;
    waveCheck(safe && stress.voices.load()==16,"Sixteen voices with all modules, EQ, pads, Doubt and Hesitation remain stable over 25 seconds");
    juce::MidiBuffer release;release.addEvent(juce::MidiMessage::allNotesOff(1),0);stress.processBlock(audio,release);
    for(int i=0;i<1400;++i){juce::MidiBuffer m;stress.processBlock(audio,m);}
    waveCheck(stress.voices.load()==0,"Sixteen sacred voices complete their Release without hanging notes");
    stress.panicRequested.store(true);
    for(int i=0;i<150;++i){juce::MidiBuffer m;stress.processBlock(audio,m);}
    waveCheck(stress.voices.load()==0 && audio.getMagnitude(0,256)==0,"Panic clears new voice modules and existing effect tails");

    // Smooth macro automation, including zero-to-maximum changes during a note.
    DroneProcessor moving;setPlain(moving,"attack",.005f);moving.prepareToPlay(48000,256);
    float previous=0,maxStep=0;bool finite=true;
    for(int block=0;block<1500;++block)
    {
        if(block%31==0){setPlain(moving,"harmony",block%2?0.0f:1.0f);setPlain(moving,"madness",block%2?1.0f:0.0f);}
        juce::MidiBuffer m;if(block==0)m.addEvent(juce::MidiMessage::noteOn(1,57,.8f),0);
        moving.processBlock(audio,m);
        for(int i=0;i<256;++i){const float x=audio.getSample(0,i);finite=finite&&std::isfinite(x);maxStep=std::max(maxStep,std::abs(x-previous));previous=x;}
    }
    waveCheck(finite && maxStep<.03f,"Harmony/Madness macro jumps are smoothed during sustained audio");
    root.getChildFile("Sacred-stress.txt").replaceWithText("CPU time ms: "+juce::String(time,3)+" / 25600 audio ms\nWorst block ms: "+juce::String(worst,3)+"\nPeak: "+juce::String(peak,6)+"\nMacro max sample delta: "+juce::String(maxStep,6)+"\n");
}
