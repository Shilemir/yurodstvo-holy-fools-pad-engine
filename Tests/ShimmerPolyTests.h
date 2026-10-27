// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include "WaveformTests.h"
#include "../Source/Shimmer.h"
#include <iomanip>

inline double toneAmplitude(const std::vector<float>& x, double frequency, double sr)
{
    double re = 0, im = 0;
    for (size_t i = 0; i < x.size(); ++i)
    {
        const double phase = juce::MathConstants<double>::twoPi * frequency * i / sr;
        re += x[i] * std::cos(phase); im += x[i] * std::sin(phase);
    }
    return 2 * std::sqrt(re*re + im*im) / x.size();
}
inline void runShimmerPolyTests(const juce::File& root)
{
    juce::String eqCsv = "sample_rate,band,gain_db,frequency_hz,measured_db\n";
    for (double sr : { 44100.0, 48000.0, 96000.0 })
    {
        for (int band = 0; band < 3; ++band) for (float db : { -12.0f, 0.0f, 12.0f })
        {
            std::array<float, 3> gains {}; gains[static_cast<size_t>(band)] = db;
            const double inBand = band == 0 ? 20 : band == 1 ? 1000 : sr * 0.40;
            for (double frequency : { inBand, band == 0 ? 15000.0 : 30.0 })
            {
                ThreeBandEQ eq; eq.prepare(sr, gains); double inputEnergy = 0, outputEnergy = 0;
                const int count = static_cast<int>(sr * 0.6);
                for (int i = 0; i < count; ++i)
                {
                    const float x = static_cast<float>(0.01 * std::sin(juce::MathConstants<double>::twoPi * frequency * i / sr));
                    const auto out = eq.process({ x, x });
                    if (i > count / 2) { inputEnergy += x*x; outputEnergy += out[0]*out[0]; }
                }
                const double measured = 10 * std::log10(outputEnergy / inputEnergy);
                const bool selected = frequency == inBand;
                waveCheck(std::abs(measured - (selected ? db : 0)) < (selected ? 0.65 : 0.2),
                    selected ? "EQ produces the actual -12/0/+12 dB gain without a hidden global trim"
                             : "EQ leaves a distant unselected band at its original level");
                eqCsv += juce::String(sr, 0) + "," + juce::String(band) + "," + juce::String(db, 1) + ","
                    + juce::String(frequency, 1) + "," + juce::String(measured, 4) + "\n";
            }
        }
        for (double frequency : { 220.0, 440.0, 1000.0 })
        {
            OctavePitchShifter pitch; pitch.prepare(sr);
            std::vector<float> shifted(static_cast<size_t>(sr * 0.5));
            for (int i = 0; i < static_cast<int>(sr); ++i)
            {
                const float x = static_cast<float>(0.1 * std::sin(juce::MathConstants<double>::twoPi * frequency * i / sr));
                const auto y = pitch.process({ x, x });
                if (i >= static_cast<int>(sr * 0.5)) shifted[static_cast<size_t>(i - static_cast<int>(sr * 0.5))] = y[0];
            }
            const double octave = toneAmplitude(shifted, 2*frequency, sr), original = toneAmplitude(shifted, frequency, sr);
            waveCheck(octave > 0.08 && octave > original * 100, "Windowed pitch shifter moves the input fundamental up exactly one octave");
        }
        for (double frequency : { 55.0, 261.625565, 333.0 })
        {
            OctavePitchShifter pitch; pitch.prepare(sr); std::vector<float> shifted(static_cast<size_t>(sr * 0.5));
            for(int i=0;i<static_cast<int>(sr);++i)
            {
                const float x=static_cast<float>(0.1*std::sin(juce::MathConstants<double>::twoPi*frequency*i/sr));
                const auto y=pitch.process({x,x});
                if(i>=static_cast<int>(sr*0.5)) shifted[static_cast<size_t>(i-static_cast<int>(sr*0.5))]=y[0];
            }
            double octaveBand=0;
            for(int offset=-12;offset<=12;++offset) octaveBand=std::max(octaveBand,toneAmplitude(shifted,2*frequency+offset,sr));
            waveCheck(octaveBand>0.015&&octaveBand>toneAmplitude(shifted,frequency,sr)*10,
                "Low and non-window-aligned notes retain octave-band energy instead of cancelling between grains");
        }
        Shimmer shimmer; shimmer.prepare(sr); bool zero = true;
        for (int i = 0; i < 2000; ++i) zero = zero && shimmer.process({ 0.2f, 0.2f }) == std::array<float,2>{0,0};
        waveCheck(zero, "Shimmer at zero has exactly zero wet output");
        shimmer.setAmount(0.5f);
        std::vector<float> wet(static_cast<size_t>(sr)); double stereoDifference = 0;
        for (int i = 0; i < static_cast<int>(3*sr); ++i)
        {
            const float x = static_cast<float>(0.2 * std::sin(juce::MathConstants<double>::twoPi * 440 * i / sr));
            const auto y = shimmer.process({ x, x });
            if (i >= static_cast<int>(2*sr)) { wet[static_cast<size_t>(i - static_cast<int>(2*sr))] = y[0]; stereoDifference += (y[0]-y[1])*(y[0]-y[1]); }
        }
        const double upper = toneAmplitude(wet, 880, sr), base = toneAmplitude(wet, 440, sr);
        std::cout << "Shimmer " << sr << " Hz: 880Hz=" << upper << " 440Hz=" << base << " stereo delta=" << stereoDifference << std::endl;
        waveCheck(upper > 0.0001 && upper > base * 10 && stereoDifference > 0.001,
            "Full Shimmer has audible octave-shifted reverb and a decorrelated stereo field");
        double firstTail = 0, finalTail = 0; bool finite = true;
        shimmer.setAmount(1);
        for (int i = 0; i < static_cast<int>(45*sr); ++i)
        {
            const auto y = shimmer.process({0,0});
            finite = finite && std::isfinite(y[0]) && std::isfinite(y[1]) && std::abs(y[0]) < 1;
            if (i < static_cast<int>(5*sr)) firstTail += y[0]*y[0] + y[1]*y[1];
            if (i >= static_cast<int>(40*sr)) finalTail += y[0]*y[0] + y[1]*y[1];
        }
        std::cout << "Shimmer tail energy, first/last 5s: " << firstTail << " / " << finalTail << std::endl;
        waveCheck(finite && firstTail > 0.001 && finalTail < firstTail * 0.0001,
            "Maximum Shimmer feedback is finite and its unforced tail decays by over 40 dB");
        shimmer.setAmount(0); for (int i = 0; i < static_cast<int>(sr * 0.1); ++i) shimmer.process({0.2f,0.2f});
        waveCheck(shimmer.process({0.2f,0.2f}) == std::array<float,2>{0,0}, "Turning Shimmer off smoothly reaches a complete bypass");
        if(sr==48000)
        {
            double previousEnergy=0;
            for(float level:{0.25f,0.5f,0.75f,1.0f})
            {
                Shimmer macro; macro.prepare(sr); macro.setAmount(level); double energy=0;
                for(int i=0;i<static_cast<int>(4*sr);++i)
                {
                    const float x=static_cast<float>(0.2*std::sin(juce::MathConstants<double>::twoPi*440*i/sr));
                    const auto y=macro.process({x,x});if(i>=static_cast<int>(3*sr))energy+=y[0]*y[0]+y[1]*y[1];
                }
                waveCheck(energy>previousEnergy*1.1,"Shimmer 25/50/75/100 percent produces progressively stronger octave ambience");
                previousEnergy=energy;
            }
        }
    }
    root.getChildFile("EQ-response-0.8.csv").replaceWithText(eqCsv);
    for (int limit : {1,4,8,16})
    {
        yuro::DroneEngine engine; engine.prepare(48000); engine.setPolyphony(limit);
        engine.setTargets({0,0.5f,0.005f,30,0.5f});
        for (int n = 0; n < limit + 5; ++n) engine.noteOn(1, 48+n, 0.9f);
        for (int i = 0; i < 4800; ++i) engine.nextSample();
        waveCheck(engine.activeVoices() == limit, "Voice stealing respects the selected 1/4/8/16 voice limit");
        engine.sustain(1, true); for (int n = 48; n < 48+limit+5; ++n) engine.noteOff(1,n);
        for (int i = 0; i < 1000; ++i) engine.nextSample();
        waveCheck(engine.activeVoices() == limit, "Sustain continues to hold voices at every polyphony limit");
        engine.setPolyphony(1); float before = engine.nextSample(), maxStep = 0;
        for (int i = 0; i < 1000; ++i) { const float x = engine.nextSample(); maxStep = std::max(maxStep, std::abs(x-before)); before=x; }
        waveCheck(engine.activeVoices() == 1 && maxStep < 0.08f, "Reducing polyphony retires excess voices with a continuous short fade");
        engine.sustain(1,false); engine.allNotesOff(0,true); for (int i = 0; i < 1000; ++i) engine.nextSample();
        waveCheck(engine.activeVoices() == 0, "Sustain release and panic leave no stuck voices after polyphony changes");
    }
    DroneProcessor state; setPlain(state,"polyphony",16); setPlain(state,"shimmer",0.63f);
    state.prepareToPlay(48000,256); juce::AudioBuffer<float> audio(2,256); juce::MidiBuffer m;
    m.addEvent(juce::MidiMessage::controllerEvent(1,86,50), 21); state.processBlock(audio,m);
    juce::MemoryBlock saved; state.getStateInformation(saved); DroneProcessor restored;
    restored.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    waveCheck(restored.polyphonyLimit() == 16 && std::abs(restored.parameters.getRawParameterValue("shimmer")->load()-50.0f/127)<0.0001f
        && restored.parameters.getParameter("shimmer")->isAutomatable() && restored.parameters.getParameter("polyphony")->isAutomatable(),
        "Shimmer CC before timer flush and polyphony are saved and automatable");
    auto legacy=state.parameters.copyState(); legacy.setProperty("schemaVersion",6,nullptr);
    legacy.removeChild(legacy.getChildWithProperty("id","shimmer"),nullptr); legacy.removeChild(legacy.getChildWithProperty("id","polyphony"),nullptr);
    juce::MemoryBlock old; auto xml=legacy.createXml(); juce::AudioProcessor::copyXmlToBinary(*xml,old);
    restored.setStateInformation(old.getData(),static_cast<int>(old.getSize()));
    waveCheck(restored.polyphonyLimit()==8 && restored.parameters.getRawParameterValue("shimmer")->load()==0,
        "Old 0.6 states load with eight voices and Shimmer off even in a reused instance");
    {
        AmbientEffects moving; moving.prepare(48000,1); float previous=0,maxStep=0;bool stable=true;
        for(int i=0;i<48000*3;++i)
        {
            if(i%400==0) moving.setShimmer((i/400)%2?0.0f:1.0f);
            const float input=0.2f*static_cast<float>(std::sin(juce::MathConstants<double>::twoPi*220*i/48000));
            const auto output=moving.process(input);maxStep=std::max(maxStep,std::abs(output[0]-previous));previous=output[0];
            stable=stable&&std::isfinite(output[0])&&std::abs(output[0])<=0.940001f;
        }
        waveCheck(stable&&maxStep<0.05f,"Rapid Shimmer automation stays bounded with continuous gain transitions");
    }
    std::unique_ptr<juce::AudioProcessorEditor> editor(state.createEditor()); juce::Slider* selector=nullptr; juce::Slider* shimmerKnob=nullptr;
    for(auto* child:editor->getChildren())
    {
        if(child->getName()=="Polyphony") selector=dynamic_cast<juce::Slider*>(child);
        if(child->getName()=="Encoder 1") shimmerKnob=dynamic_cast<juce::Slider*>(child);
    }
    waveCheck(selector && selector->getValue()==16 && shimmerKnob && shimmerKnob->isEnabled(), "GUI exposes the saved polyphony and active E1 Shimmer");
    selector->setValue(3,juce::sendNotificationSync); shimmerKnob->setValue(0.37,juce::sendNotificationSync);
    waveCheck(state.polyphonyLimit()==3 && std::abs(state.parameters.getRawParameterValue("shimmer")->load()-0.37f)<0.001,
        "Mouse polyphony and Shimmer edit their host parameters");
    DroneProcessor full; setPlain(full,"polyphony",16); setPlain(full,"shimmer",1); setPlain(full,"reverb",1); setPlain(full,"delay",1);
    setPlain(full,"eqBass",12); setPlain(full,"eqMid",12); setPlain(full,"eqTreble",12); setPlain(full,"attack",0.005f); setPlain(full,"release",30);
    full.prepareToPlay(48000,256); bool safe=true; double total=0,worst=0; float peak=0;
    for(int block=0;block<1500;++block)
    {
        juce::MidiBuffer midi;
        if(block==0) for(int n=0;n<16;++n) midi.addEvent(juce::MidiMessage::noteOn(1,48+n*2,1.0f),0);
        if(block%11==0)
        {
            midi.addEvent(juce::MidiMessage::controllerEvent(1,102+block%8,127),32);
            midi.addEvent(juce::MidiMessage::pitchWheel(1,block%2?0:16383),64);
            midi.addEvent(juce::MidiMessage::controllerEvent(1,1,block%128),95);
        }
        if(block>300&&block%37==0)
        {
            midi.addEvent(juce::MidiMessage::controllerEvent(1,15,block%128),110);
            midi.addEvent(juce::MidiMessage::controllerEvent(1,30,(block*3)%128),130);
            midi.addEvent(juce::MidiMessage::controllerEvent(1,31,(block*7)%128),150);
        }
        const double start=juce::Time::getMillisecondCounterHiRes(); full.processBlock(audio,midi);
        const double elapsed=juce::Time::getMillisecondCounterHiRes()-start; total+=elapsed; worst=std::max(worst,elapsed);
        for(int ch=0;ch<2;++ch) for(int i=0;i<256;++i)
        { const float x=audio.getSample(ch,i); safe=safe&&std::isfinite(x)&&std::abs(x)<=0.940001f; peak=std::max(peak,std::abs(x)); }
    }
    const double audioMs=1500*256/48.0;
    std::cout<<"16 voices + EQ + Shimmer + Reverb + Delay: "<<total<<" ms per "<<audioMs<<" ms audio, average "<<100*total/audioMs<<"%, worst 256-sample block "<<worst<<" ms, peak "<<peak<<std::endl;
    waveCheck(safe&&full.voices.load()==16, "Sixteen held voices, +12 dB EQ, all effects and controller changes stay bounded");
    juce::MidiBuffer off; off.addEvent(juce::MidiMessage::allNotesOff(1),0); full.processBlock(audio,off);
    for(int i=0;i<100;++i) {juce::MidiBuffer empty; full.processBlock(audio,empty);}
    waveCheck(full.voices.load()==16,"Sixteen voices retain their long Release after Note Off");
    setPlain(full,"master",0);float muted=0;
    for(int i=0;i<100;++i) {juce::MidiBuffer empty;full.processBlock(audio,empty);if(i>50)muted=std::max(muted,audio.getMagnitude(0,256));}
    waveCheck(muted==0,"F1 Master zero mutes Shimmer as well as EQ, Reverb, Delay and sixteen release tails");
    setPlain(full,"master",1);
    full.panicRequested.store(true); float residue=0;
    for(int i=0;i<400;++i) {juce::MidiBuffer empty; full.processBlock(audio,empty); if(i>200) residue=std::max(residue,audio.getMagnitude(0,256));}
    waveCheck(full.voices.load()==0&&residue<0.000001f,"Panic clears all sixteen voices, EQ, Shimmer, Reverb and Delay");
}

inline void compareNominalVSTLevels(const juce::File& root, const juce::String& current, const juce::String& previous)
{
    auto measure = [](const juce::String& path)
    {
        juce::VST3PluginFormat format; juce::OwnedArray<juce::PluginDescription> descriptions; format.findAllTypesForFile(descriptions,path);
        if(descriptions.isEmpty()) throw std::runtime_error("Cannot scan nominal comparison VST3");
        juce::String error; auto instance=format.createInstanceFromDescription(*descriptions[0],48000,512,error);
        if(!instance) throw std::runtime_error(error.toStdString());
        for(auto* p:instance->getParameters())
        {
            const auto name=p->getName(80);
            if(name=="Attack"||name=="Reverb"||name=="Delay"||name=="Shimmer") p->setValueNotifyingHost(0);
            if(name=="Oscillator Mix"||name=="Master Volume"||name=="Pitch Detune"||name=="Oscillator Detune"||name=="Bass"||name=="Mid"||name=="Treble") p->setValueNotifyingHost(0.5f);
        }
        instance->prepareToPlay(48000,512); juce::AudioBuffer<float> audio(2,512);
        for(int i=0;i<100;++i){juce::MidiBuffer m;instance->processBlock(audio,m);}
        double energy=0,peak=0; int count=0;
        for(int block=0;block<240;++block)
        {
            juce::MidiBuffer m; if(block==0) for(int n:{60,64,67,71}) m.addEvent(juce::MidiMessage::noteOn(1,n,0.8f),0);
            instance->processBlock(audio,m);
            if(block<50)continue;
            for(int i=0;i<512;++i) { const double x=audio.getSample(0,i);energy+=x*x;peak=std::max(peak,std::abs(x));++count; }
        }
        instance->releaseResources();return std::array<double,2>{std::sqrt(energy/count),peak};
    };
    const auto before=measure(previous),after=measure(current);
    const double rmsDb=20*std::log10(after[0]/before[0]),peakDb=20*std::log10(after[1]/before[1]);
    std::cout<<"Actual VST3 neutral chord 0.6 RMS="<<before[0]<<" peak="<<before[1]<<"; 0.8 RMS="<<after[0]<<" peak="<<after[1]<<"; change RMS="<<rmsDb<<" dB, peak="<<peakDb<<" dB"<<std::endl;
    waveCheck(std::abs(rmsDb-3)<0.01&&std::abs(peakDb-3)<0.01,"Actual 0.6 and 0.8 VST3 neutral chord RMS and peak both increase by 3 dB");
    root.getChildFile("Nominal-levels-0.8.csv").replaceWithText("version,rms_linear,peak_linear\n0.6,"+juce::String(before[0],9)+","+juce::String(before[1],9)+"\n0.8,"+juce::String(after[0],9)+","+juce::String(after[1],9)+"\n");
}
