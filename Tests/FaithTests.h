// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "SacredTests.h"

inline void runFaithTests(const juce::File& root,const juce::String& currentPath)
{
    DroneProcessor a,b;
    for(auto* p:{&a,&b})
    {
        setPlain(*p,"attack",.005f);setPlain(*p,"madness",1);setPlain(*p,"harmony",.7f);
        setPlain(*p,"shimmer",.5f);setPlain(*p,"reverb",.5f);setPlain(*p,"delay",.5f);p->prepareToPlay(48000,256);
    }
    juce::AudioBuffer<float> x(2,256),y(2,256);bool identical=true;
    for(int block=0;block<1600;++block)
    {
        juce::MidiBuffer m;
        if(block==0)for(int n:{48,55,60,64})m.addEvent(juce::MidiMessage::noteOn(1,n,.8f),0);
        auto other=m;other.addEvent(juce::MidiMessage::controllerEvent(1,1,block%128),block%256);
        a.processBlock(x,m);b.processBlock(y,other);
        for(int ch=0;ch<2;++ch)for(int i=0;i<256;++i)identical=identical && x.getSample(ch,i)==y.getSample(ch,i);
    }
    waveCheck(identical && b.modulation.load()==1599%128,"Moving physical FAITH changes its visual state with sample-exact zero effect on sound");
    std::unique_ptr<juce::AudioProcessorEditor> editor(b.createEditor());juce::Slider* faith=nullptr;juce::Slider* poly=nullptr;
    for(auto* c:editor->getChildren())
    {
        if(c->getName()=="Mod Wheel")faith=dynamic_cast<juce::Slider*>(c);
        if(c->getName()=="Polyphony")poly=dynamic_cast<juce::Slider*>(c);
    }
    waveCheck(faith && poly,"Faith strip and polyphony selector remain in the editor");
    faith->setValue(127,juce::sendNotificationSync);
    waveCheck(b.modulation.load()==127,"Mouse FAITH updates the same visual CC1 position");
    for(int block=0;block<100;++block)
    {
        juce::MidiBuffer m,n;a.processBlock(x,m);b.processBlock(y,n);
        for(int ch=0;ch<2;++ch)for(int i=0;i<256;++i)identical=identical&&x.getSample(ch,i)==y.getSample(ch,i);
    }
    waveCheck(identical,"Mouse FAITH also leaves the complete audio path sample-exact");
    bool buttons=true;int count=0;
    for(auto* c:poly->getChildren())if(auto* button=dynamic_cast<juce::Button*>(c))
    {
        poly->setValue(8,juce::dontSendNotification);
        if(button->onClick)button->onClick();
        buttons=buttons && std::abs(poly->getValue()-8)==1;++count;
    }
    waveCheck(buttons && count==2,"Both original polyphony increment/decrement buttons still change the value");

    juce::String paths[4];
    for(float scale:{.75f,1.0f,2.0f})for(int shape=0;shape<4;++shape)
    {
        const juce::Rectangle<float> bounds(10*scale,44*scale,75*scale,18*scale);
        auto path=WavePad::waveformPath(shape,bounds);
        int moves=0,lines=0;juce::Path::Iterator iterator(path);
        while(iterator.next()){if(iterator.elementType==juce::Path::Iterator::startNewSubPath)++moves;else if(iterator.elementType==juce::Path::Iterator::lineTo)++lines;}
        waveCheck(moves==1 && lines>=3 && bounds.expanded(.001f).contains(path.getBounds())
            && path.getBounds().getWidth()>bounds.getWidth()*.99f && path.getBounds().getHeight()>bounds.getHeight()*.99f,
            "Each waveform is one complete vector path, spanning its drawing area without clipping at multiple scales");
        if(scale==1)paths[shape]=path.toString();
    }
    waveCheck(paths[0]!=paths[1] && paths[1]!=paths[2] && paths[2]!=paths[3],"All four waveform symbols are distinct vector geometry");
    DroneLookAndFeel look;juce::TextButton button("+");button.setSize(44,28);
    std::array<juce::Colour,4> colours;
    juce::Image sheet(juce::Image::RGB,176,28,true);
    {
    juce::Graphics sheetGraphics(sheet);
    for(int state=0;state<4;++state)
    {
        juce::Image picture(juce::Image::RGB,44,28,true);
        button.setEnabled(state!=3);
        {juce::Graphics g(picture);look.drawButtonBackground(g,button,juce::Colours::blue,state==1,state==2);}
        colours[static_cast<size_t>(state)]=picture.getPixelAt(10,10);
        {juce::Graphics g(picture);look.drawButtonText(g,button,state==1,state==2);}
        sheetGraphics.drawImageAt(picture,state*44,0);
    }
    }
    waveCheck(colours[0]!=colours[1] && colours[1]!=colours[2] && colours[2]!=colours[3],"Button normal, hover, pressed and disabled states have distinct warm colours");
    for(auto colour:colours)waveCheck(colour.getRed()>=colour.getBlue() && colour.getGreen()>=colour.getBlue(),"Button state ignores default blue and uses the warm panel palette");
    auto png=root.getChildFile("Polyphony-button-states.png").createOutputStream();juce::PNGImageFormat().writeImageToStream(sheet,*png);

    const auto previousPath=root.getChildFile("0.9.0/VST3/YURODSTVO - Holy Fool's Pad Engine.vst3");
    if(!previousPath.exists()){std::cout<<"SKIP: optional private 0.9.0 Madness comparison unavailable\n";return;}
    juce::VST3PluginFormat format;
    auto load=[&](const juce::String& path)
    {
        juce::OwnedArray<juce::PluginDescription> descriptions;format.findAllTypesForFile(descriptions,path);
        if(descriptions.isEmpty())throw std::runtime_error("Cannot scan Madness comparison plugin");
        juce::String error;auto result=format.createInstanceFromDescription(*descriptions[0],48000,256,error);
        if(!result)throw std::runtime_error(error.toStdString());return result;
    };
    for(float amount:{0.0f,.25f,.5f,.75f,1.0f})
    {
        auto old=load(previousPath.getFullPathName()),now=load(currentPath);
        for(auto* p:{old.get(),now.get()})
        {
            for(auto* parameter:p->getParameters())
            {
                const auto name=parameter->getName(80);
                if(name=="Madness")parameter->setValueNotifyingHost(amount);
                if(name=="Polyphony" || name=="Master Volume")parameter->setValueNotifyingHost(1);
                if(name=="Attack")parameter->setValueNotifyingHost(0);
                if(name=="Harmony")parameter->setValueNotifyingHost(0);
                if(name=="Shimmer" || name=="Reverb" || name=="Delay")parameter->setValueNotifyingHost(.5f);
            }
            p->prepareToPlay(48000,256);
            for(int block=0;block<64;++block)
            {
                juce::MidiBuffer m;if(block==0)m.addEvent(juce::MidiMessage::controllerEvent(1,1,127),0);
                p->processBlock(x,m); // Settle old expression smoothing before the first note.
            }
        }
        float error=0;
        for(int block=0;block<1800;++block)
        {
            juce::MidiBuffer m;
            if(block==0)for(int n=0;n<16;++n)m.addEvent(juce::MidiMessage::noteOn(1,48+n*2,.8f),0);
            if(block==600)m.addEvent(juce::MidiMessage::pitchWheel(1,16383),13);
            if(block==900)m.addEvent(juce::MidiMessage::pitchWheel(1,8192),13);
            if(block==1200)m.addEvent(juce::MidiMessage::allNotesOff(1),0);
            auto copy=m;copy.addEvent(juce::MidiMessage::controllerEvent(1,1,block%128),37);
            old->processBlock(x,m);now->processBlock(y,copy);
            for(int ch=0;ch<2;++ch)for(int i=0;i<256;++i)error=std::max(error,std::abs(x.getSample(ch,i)-y.getSample(ch,i)));
        }
        std::cout<<"Madness "<<amount<<": 0.9.0 CC1=127 vs new changing FAITH max sample error="<<error<<std::endl;
        waveCheck(error<1e-6,"New E8 equals actual 0.9.0 at full Hesitation, with sixteen voices, all FX, global pitch and Release");
        old->releaseResources();now->releaseResources();
    }
}
