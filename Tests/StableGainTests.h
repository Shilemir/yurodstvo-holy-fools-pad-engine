// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "WaveformTests.h"

inline void runStableGainTests(const juce::File& root,const juce::String& currentPath)
{
    const auto oldPath=root.getChildFile("0.15.0/VST3/YURODSTVO - Holy Fool's Synth for Arturia MiniLab3.vst3");
    if(!oldPath.exists() || currentPath.isEmpty())
    { std::cout << "SKIP: optional 0.15.0 VST3 Harmony comparison binary unavailable\n";return; }
    juce::VST3PluginFormat format;
    auto make=[&](const juce::String& path,bool harmony)
    {
        juce::OwnedArray<juce::PluginDescription> found;format.findAllTypesForFile(found,path);
        if(found.isEmpty())throw std::runtime_error("Cannot scan Harmony baseline VST3");
        juce::String error;
        auto instance=format.createInstanceFromDescription(*found[0],48000,256,error);
        if(!instance)throw std::runtime_error(error.toStdString());
        for(auto* parameter:instance->getParameters())
        {
            const auto name=parameter->getName(80);
            if(name=="Attack" || name=="Reverb" || name=="Delay" || name=="Shimmer" || name=="Madness")
                parameter->setValueNotifyingHost(0);
            else if(name=="Harmony")parameter->setValueNotifyingHost(harmony?1.0f:0.0f);
            else if(name=="Master Volume")parameter->setValueNotifyingHost(.08f);
            else if(name=="Oscillator Mix")parameter->setValueNotifyingHost(.5f);
        }
        instance->prepareToPlay(48000,256);return instance;
    };
    auto oldDry=make(oldPath.getFullPathName(),false);
    auto oldWet=make(oldPath.getFullPathName(),true);
    auto newDry=make(currentPath,false);
    auto newWet=make(currentPath,true);
    std::array<juce::AudioBuffer<float>,4> buffers {
        juce::AudioBuffer<float>(2,256),juce::AudioBuffer<float>(2,256),
        juce::AudioBuffer<float>(2,256),juce::AudioBuffer<float>(2,256) };
    struct Window { double energy=0,cross=0,residual=0; int count=0; };
    Window onset, recurring;
    float dryError=0,peak=0;
    constexpr double target=1.25892541179;
    for(int block=0;block<3750;++block)
    {
        juce::MidiBuffer midi;
        if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,57,.65f),0);
        auto m1=midi,m2=midi,m3=midi;
        oldDry->processBlock(buffers[0],midi);
        oldWet->processBlock(buffers[1],m1);
        newDry->processBlock(buffers[2],m2);
        newWet->processBlock(buffers[3],m3);
        for(int i=0;i<256;++i)
        {
            const int sample=block*256+i;
            const double before=buffers[1].getSample(0,i)-buffers[0].getSample(0,i);
            const double after=buffers[3].getSample(0,i)-buffers[2].getSample(0,i);
            dryError=std::max(dryError,std::abs(buffers[2].getSample(0,i)-buffers[0].getSample(0,i)));
            peak=std::max(peak,std::abs(buffers[3].getSample(0,i)));
            Window* window=sample>=9600&&sample<86400 ? &onset
                : sample>=480000&&sample<864000 ? &recurring : nullptr;
            if(window){window->energy+=before*before;window->cross+=before*after;
                const double error=after-target*before;window->residual+=error*error;++window->count;}
        }
    }
    const auto pass=[&](const Window& window)
    {
        const double gain=window.cross/window.energy;
        const double residual=std::sqrt(window.residual/window.energy);
        std::cout << "Harmony 1.0 gain=" << gain << " residual=" << residual
            << " samples=" << window.count << std::endl;
        return window.energy>1e-7 && std::abs(20*std::log10(gain)-2.0)<.01 && residual<.003;
    };
    waveCheck(dryError<1e-6f,"Dry oscillator output matches actual 0.15.0 VST3 exactly");
    waveCheck(pass(onset),"Initial Harmony return is exactly +2 dB against 0.15.0");
    waveCheck(pass(recurring),"Recurring Harmony return is exactly +2 dB against 0.15.0");
    waveCheck(peak<.75f,"Measured gain comparison remains below the final output safety knee");
    for(auto* instance:{oldDry.get(),oldWet.get(),newDry.get(),newWet.get()})instance->releaseResources();
}
