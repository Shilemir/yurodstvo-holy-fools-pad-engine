// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "WaveformTests.h"
#include "../Source/PluginEditor.h"
#include "../Source/AmbientEffects.h"
#include <limits>

inline void runRefinementTests(const juce::File& root,const juce::String& currentPath)
{
    DroneProcessor processor;neutralTestSound(processor);
    waveCheck(processor.parameters.getRawParameterValue("mix")->load()==.5f,
        "New Dualism instances retain the equal 0.5/0.5 linear oscillator balance");
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    std::array<juce::Slider*,8> knobs {};
    juce::TextButton *pray=nullptr,*silence=nullptr;
    for(auto* child:editor->getChildren())
    {
        for(int i=0;i<8;++i)if(child->getName()=="Encoder "+juce::String(i+1))knobs[static_cast<size_t>(i)]=dynamic_cast<juce::Slider*>(child);
        if(child->getName()=="Pray for Inspiration")pray=dynamic_cast<juce::TextButton*>(child);
        if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()=="SILENCE")silence=button;
    }
    waveCheck(knobs[1]&&knobs[1]->getTextFromValue(0)=="-100%"
        &&knobs[1]->getTextFromValue(.5)=="0%"&&knobs[1]->getTextFromValue(1)=="+100%"
        &&knobs[1]->getTextFromValue(.62)=="+24%"
        &&std::abs(knobs[1]->getValueFromText("-50%")-.25)<.0001,
        "Dualism displays a bipolar balance while retaining the original 0-1 parameter scale");
    bool percentages=true;
    for(int index:{0,4,5,6,7})percentages=percentages&&knobs[static_cast<size_t>(index)]
        &&knobs[static_cast<size_t>(index)]->getTextFromValue(.125)=="12.5%"
        &&knobs[static_cast<size_t>(index)]->getTextFromValue(0)=="0.0%"
        &&knobs[static_cast<size_t>(index)]->getTextFromValue(1)=="100.0%";
    waveCheck(percentages,"Five effect macros display one-decimal percentages without changing parameter precision");
    waveCheck(pray&&silence&&pray->findColour(juce::TextButton::textColourOffId)
        ==silence->findColour(juce::TextButton::textColourOffId),
        "Prayer and Silence use the identical warm-gold text colour");
    setPlain(processor,"mix",.62f);juce::MemoryBlock state;processor.getStateInformation(state);
    DroneProcessor restored;neutralTestSound(restored);restored.setStateInformation(state.getData(),static_cast<int>(state.getSize()));
    waveCheck(std::abs(restored.parameters.getRawParameterValue("mix")->load()-.62f)<.0001f,
        "A legacy positive Dualism balance survives the host-state round trip unchanged");
    float minimum=1,maximum=0;
    for(int i=0;i<128;++i){processor.randomizeSound();const auto mix=processor.parameters.getRawParameterValue("mix")->load();minimum=std::min(minimum,mix);maximum=std::max(maximum,mix);}
    waveCheck(minimum<.25f&&maximum>.75f,"Pray for Inspiration explores both polarities of Dualism");

    const auto burstRms=[](float strength)
    {
        yuro::SacredResonance modes;modes.reset(false);modes.strike(strength,.2f,48000);
        double energy=0;for(int i=0;i<4800;++i){const auto x=modes.process(0,220,48000,1);energy+=x*x;}
        return std::sqrt(energy/4800);
    };
    const double bellDb=20*std::log10(burstRms(.8f*2.35f)/burstRms(.8f));
    waveCheck(std::abs(bellDb-7.42)<.05,"Recurring modal excitation is about 7.4 dB stronger than its former strength");

    const auto impulseAt=[&](double bpm)
    {
        AmbientEffects fx;fx.prepare(48000,1);fx.setTargets(0,1,1);fx.setTempoBpm(bpm);
        const int last=static_cast<int>(48000*1.5*60.0/(std::isfinite(bpm)&&bpm>=30&&bpm<=300?bpm:120))+20;
        std::array<float,2> peaks {};std::array<int,2> times {};
        for(int n=0;n<last;++n)
        {
            const auto out=fx.process(n==0?.1f:0.0f);
            if(n<100)continue;
            for(int ch=0;ch<2;++ch)if(std::abs(out[static_cast<size_t>(ch)])>peaks[static_cast<size_t>(ch)])
            {peaks[static_cast<size_t>(ch)]=std::abs(out[static_cast<size_t>(ch)]);times[static_cast<size_t>(ch)]=n;}
        }
        return times;
    };
    for(double bpm:{60.0,90.0,120.0,150.0})
    {
        const auto times=impulseAt(bpm);
        waveCheck(std::abs(times[0]-juce::roundToInt(48000*60/bpm))<=1
            &&std::abs(times[1]-juce::roundToInt(48000*90/bpm))<=1,
            "Stereo Solitude impulse taps follow quarter and dotted-quarter note timing");
    }
    const auto fallback=impulseAt(std::numeric_limits<double>::quiet_NaN());
    waveCheck(fallback[0]==24000&&fallback[1]==36000,
        "Invalid or absent host BPM keeps the 120 BPM standalone default");
    AmbientEffects moving;moving.prepare(48000,1);moving.setTargets(0,1,1);moving.setTempoBpm(90);
    float previous=0,maximumStep=0,maximumPeak=0;bool finite=true;
    for(int n=0;n<48000*4;++n)
    {
        if(n==48000*2)moving.setTempoBpm(120);
        const float input=.04f*std::sin(juce::MathConstants<float>::twoPi*110*n/48000.0f);
        const float out=moving.process(input)[0];finite=finite&&std::isfinite(out);
        maximumStep=std::max(maximumStep,std::abs(out-previous));maximumPeak=std::max(maximumPeak,std::abs(out));previous=out;
    }
    waveCheck(finite&&maximumStep<.15f&&maximumPeak<=.940001f,
        "A live 90-to-120 BPM change crossfades delay taps without an unsafe discontinuity");

    const auto oldPath=root.getChildFile("0.12.0/VST3/YURODSTVO - Holy Fool's Synth for Arturia MiniLab3.vst3");
    if(!oldPath.exists()||currentPath.isEmpty())return;
    juce::VST3PluginFormat format;
    auto load=[&](const juce::String& path)
    {
        juce::OwnedArray<juce::PluginDescription> descriptions;format.findAllTypesForFile(descriptions,path);
        if(descriptions.isEmpty())throw std::runtime_error("Cannot scan Harmony comparison plugin");
        juce::String error;auto instance=format.createInstanceFromDescription(*descriptions[0],48000,256,error);if(instance)neutralHostSound(*instance);
        if(!instance)throw std::runtime_error(error.toStdString());
        for(auto* parameter:instance->getParameters())
        {
            const auto name=parameter->getName(80);
            if(name=="Attack"||name=="Harmony")parameter->setValueNotifyingHost(1);
            if(name=="Master Volume"||name=="Oscillator Mix")parameter->setValueNotifyingHost(.5f);
        }
        instance->prepareToPlay(48000,256);return instance;
    };
    auto old=load(oldPath.getFullPathName()),now=load(currentPath);
    juce::AudioBuffer<float> a(2,256),b(2,256);
    float initialError=0,peak=0;double laterEnergy=0;int laterCount=0;
    for(int block=0;block<4000;++block)
    {
        juce::MidiBuffer midi;if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,57,.8f),0);
        auto copy=midi;old->processBlock(a,midi);now->processBlock(b,copy);
        for(int i=0;i<256;++i)
        {
            const float difference=b.getSample(0,i)-a.getSample(0,i);
            if(block<375)initialError=std::max(initialError,std::abs(difference));
            if(block>750){laterEnergy+=difference*difference;++laterCount;}
            peak=std::max(peak,std::abs(b.getSample(0,i)));
        }
    }
    const double laterDifference=std::sqrt(laterEnergy/laterCount);
    std::cout<<"Harmony comparison: onset max error="<<initialError<<", later RMS difference="<<laterDifference<<", peak="<<peak<<std::endl;
    waveCheck(initialError>0&&laterDifference>.001&&peak<=.940001f,
        "Against 0.12.0, both initial and recurring Harmony output strengthen safely");
    old->releaseResources();now->releaseResources();
}
