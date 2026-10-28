// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "WaveformTests.h"
#include "../Source/PluginEditor.h"

inline void runArchetypeTests(const juce::File& root)
{
    const std::array<yuro::SoundPreset,4> specified {{
        {{.184f,.3162f,4,8,.541f,.15f,.33f,.08f,0,0,0,0,1}},
        {{.054f,.3605f,1.011f,13.905f,.237f,.498f,1,1,-.6f,.3f,6.6f,1,0}},
        {{1,.2045f,.005f,1.115f,.35f,0,1,.666f,-7.5f,0,2.9f,3,0}},
        {{.80f,.50f,3,15,.90f,.35f,1,.15f,-1,2,3,1,0}}
    }};
    DroneProcessor processor;neutralTestSound(processor);
    setPlain(processor,"master",.73f);setPlain(processor,"polyphony",12);
    processor.setPitchFromMidi(12000);processor.setModulation(85);
    for(size_t slot=0;slot<yuro::archetypes.size();++slot)
    {
        processor.applySoundPreset(yuro::archetypes[slot]);
        const auto sound=processor.captureSound();bool matching=true;
        for(size_t i=0;i<sound.values.size();++i)
            matching=matching&&std::abs(sound.values[i]-specified[slot].values[i])<.0002f;
        waveCheck(matching,"Factory archetype sets all eleven controls and both waveforms");
        waveCheck(std::abs(processor.parameters.getRawParameterValue("master")->load()-.73f)<.001f
            &&processor.polyphonyLimit()==12 &&processor.currentDetune()>0 &&processor.modulation.load()==85,
            "Factory archetype preserves Master, polyphony, Doubt and Faith");
    }
    waveCheck(yuro::archetypes[0].values[2]>yuro::archetypes[1].values[2]
        &&yuro::archetypes[2].values[2]<.05f
        &&yuro::archetypes[1].values[7]>.9f
        &&yuro::archetypes[2].values[6]>.9f
        &&yuro::archetypes[3].values[0]>.7f,
        "Four archetypes have distinctly slow, unstable, percussive and spatial settings");
    std::array<double,4> earlyRms {}, brightness {};
    for(size_t slot=0;slot<4;++slot)
    {
        DroneProcessor sound;neutralTestSound(sound);setPlain(sound,"master",.65f);
        sound.applySoundPreset(yuro::archetypes[slot]);sound.prepareToPlay(48000,256);
        juce::AudioBuffer<float> blockAudio(2,256);double energy=0,difference=0;
        float previous=0;int count=0;
        for(int block=0;block<188;++block)
        {
            juce::MidiBuffer midi;if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,60,.8f),0);
            sound.processBlock(blockAudio,midi);
            for(int i=0;i<256;++i)
            {
                const int sample=block*256+i;const float value=blockAudio.getSample(0,i);
                if(sample>=9600 && sample<38400){energy+=value*value;
                    const auto delta=value-previous;difference+=delta*delta;++count;}
                previous=value;
            }
        }
        earlyRms[slot]=std::sqrt(energy/count);
        brightness[slot]=difference/juce::jmax(energy,1e-12);
    }
    waveCheck(earlyRms[2]>earlyRms[0]*3 && earlyRms[1]>earlyRms[0]*3,
        "Equal-note render distinguishes Hermit's slow entrance from Martyr and Inquisitor attacks");
    waveCheck(brightness[2]>brightness[0]*1.5,
        "Equal-note render gives Inquisitor measurably brighter, more metallic early content than Hermit");
    const auto directory=root.getChildFile("CustomPresetTest");
    yuro::CustomPresetStore store(directory);yuro::SoundPreset loaded;
    waveCheck(store.location().getFileName()=="custom-preset-v1.json", "THE FOOL retains the previous CUSTOM storage format and filename");
    store.location().deleteFile();
    waveCheck(!store.load(loaded),"An unsaved Custom slot reports empty");
    processor.applySoundPreset(yuro::archetypes[1]);
    const auto saved=processor.captureSound();
    waveCheck(store.save(saved),"Custom preset writes a versioned user-data JSON atomically");
    yuro::CustomPresetStore reopened(directory);
    waveCheck(reopened.load(loaded),"Custom preset survives a fresh store instance");
    processor.applySoundPreset(yuro::archetypes[0]);processor.applySoundPreset(loaded);
    bool restored=true;const auto current=processor.captureSound();
    for(size_t i=0;i<current.values.size();++i)restored=restored&&std::abs(current.values[i]-saved.values[i])<.012f;
    waveCheck(restored&&std::abs(processor.parameters.getRawParameterValue("master")->load()-.73f)<.001f,
        "Custom restores only the thirteen sound values, leaving Master untouched");
    processor.applySoundPreset(yuro::archetypes[0]);
    juce::MemoryBlock session;processor.getStateInformation(session);
    DroneProcessor hostRestored;neutralTestSound(hostRestored);hostRestored.setStateInformation(session.getData(),static_cast<int>(session.getSize()));
    waveCheck(std::abs(hostRestored.parameters.getRawParameterValue("madness")->load()-.08f)<.001f,
        "Host project restores its own sound without automatically loading global Custom");
    store.location().replaceWithText("{\"version\":99}");
    waveCheck(!store.load(loaded),"Corrupt or incompatible Custom is rejected without replacing the current sound");

    auto send=[&](DroneProcessor& p,int cc,int value)
    {juce::AudioBuffer<float> audio(2,256);juce::MidiBuffer m;
        m.addEvent(juce::MidiMessage::controllerEvent(1,cc,value),0);p.processBlock(audio,m);p.flushMidiParameters();};
    for(int encoder=0;encoder<8;++encoder)
    {
        DroneProcessor p;neutralTestSound(p);p.prepareToPlay(48000,256);
        const int control=encoder<6?encoder:encoder+4;
        const int cc=yuro::controlCC[static_cast<size_t>(control)];
        send(p,cc,50);p.applySoundPreset(yuro::archetypes[0]);
        const auto* parameter=p.parameters.getParameter(yuro::soundIds[static_cast<size_t>(encoder)]);
        const float start=parameter->getValue();
        send(p,cc,51);
        waveCheck(std::abs(parameter->getValue()-juce::jmin(1.0f,start+1.0f/127))<.009f,
            "E1-E8 respond to the first known positive physical CC step after a preset");
        const float forward=parameter->getValue();send(p,cc,50);
        waveCheck(parameter->getValue()<=forward+.001f,
            "E1-E8 reverse without pickup travel or a large jump");
    }
    DroneProcessor unknown;neutralTestSound(unknown);unknown.prepareToPlay(48000,256);unknown.applySoundPreset(yuro::archetypes[1]);
    const float baseline=unknown.parameters.getRawParameterValue("mix")->load();
    send(unknown,71,80);waveCheck(unknown.parameters.getRawParameterValue("mix")->load()==baseline,
        "Unknown initial absolute encoder position establishes a safe baseline");
    send(unknown,71,81);waveCheck(unknown.parameters.getRawParameterValue("mix")->load()>baseline,
        "Second observation after unknown baseline responds in the correct direction");
    DroneProcessor eqFader;neutralTestSound(eqFader);eqFader.prepareToPlay(48000,256);eqFader.applySoundPreset(yuro::archetypes[1]);
    const auto eqStart=eqFader.parameters.getRawParameterValue("eqBass")->load();
    send(eqFader,82,0);
    waveCheck(eqFader.parameters.getRawParameterValue("eqBass")->load()==eqStart,
        "A distant absolute EQ fader cannot jump immediately after selecting a sound");
    send(eqFader,82,juce::roundToInt((eqStart+yuro::maxEqDb)/(2*yuro::maxEqDb)*127));
    send(eqFader,82,juce::jmin(127,juce::roundToInt((eqStart+yuro::maxEqDb)/(2*yuro::maxEqDb)*127)+1));
    waveCheck(eqFader.parameters.getRawParameterValue("eqBass")->load()!=eqStart,
        "EQ fader resumes absolute control after crossing its displayed preset value");
    DroneProcessor faders;neutralTestSound(faders);faders.prepareToPlay(48000,256);send(faders,17,40);
    faders.applySoundPreset(yuro::archetypes[2]);
    const float master=faders.parameters.getRawParameterValue("master")->load();
    send(faders,17,41);waveCheck(faders.parameters.getRawParameterValue("master")->load()!=master,
        "Selecting a sound leaves the unchanged physical Master responsive");
    faders.armFaderPickup(3);const float guarded=faders.parameters.getRawParameterValue("master")->load();
    send(faders,17,120);waveCheck(faders.parameters.getRawParameterValue("master")->load()==guarded,
        "A mouse-edited Master cannot jump to a distant physical fader position");

    DroneProcessor playing;neutralTestSound(playing);setPlain(playing,"polyphony",16);playing.prepareToPlay(48000,256);
    juce::AudioBuffer<float> audio(2,256);bool safe=true;float peak=0;
    for(int block=0;block<800;++block)
    {
        if(block%100==0)playing.applySoundPreset(yuro::archetypes[static_cast<size_t>((block/100)%4)]);
        juce::MidiBuffer m;if(block==0)for(int note=48;note<80;note+=2)
            m.addEvent(juce::MidiMessage::noteOn(1,note,.8f),0);
        playing.processBlock(audio,m);
        for(int channel=0;channel<2;++channel)for(int sample=0;sample<256;++sample)
        {const float value=audio.getSample(channel,sample);safe=safe&&std::isfinite(value);
            peak=std::max(peak,std::abs(value));}
    }
    waveCheck(safe&&peak<=.940001f&&playing.voices.load()>0,
        "Switching archetypes during sixteen held voices keeps finite bounded output and voices alive");
    DroneProcessor guiProcessor;neutralTestSound(guiProcessor);std::unique_ptr<juce::AudioProcessorEditor> editor(guiProcessor.createEditor());
    std::array<juce::TextButton*,4> factoryButtons {};
    juce::TextButton *customButton=nullptr,*saveButton=nullptr,*prayer=nullptr,*silence=nullptr;
    for(auto* component:editor->getChildren())
        if(auto* button=dynamic_cast<juce::TextButton*>(component))
        {
            for(size_t i=0;i<4;++i)if(button->getName()=="Factory Preset "+juce::String(static_cast<int>(i+1)))factoryButtons[i]=button;
            if(button->getButtonText()=="THE FOOL (CUSTOM)")customButton=button;
            if(button->getButtonText()=="SAVE")saveButton=button;
            if(button->getButtonText()=="PRAY FOR INSPIRATION")prayer=button;
            if(button->getButtonText()=="SILENCE")silence=button;
        }
    bool arranged=customButton&&saveButton&&prayer&&silence;
    for(const auto* button:factoryButtons)arranged=arranged&&button&&customButton&&button->getBottom()<=customButton->getY();
    waveCheck(arranged&&customButton->getBottom()<prayer->getY()&&prayer->getBottom()<silence->getY(),
        "Four archetypes, Custom, Save, Prayer and Silence fit the left panel without overlap");
    auto* droneEditor=dynamic_cast<DroneEditor*>(editor.get());
    bool buttonsWork=droneEditor!=nullptr;
    for(size_t slot=0;slot<factoryButtons.size();++slot)
    {
        factoryButtons[slot]->onClick();
        const auto shown=guiProcessor.captureSound();
        for(size_t i=0;i<shown.values.size();++i)
            buttonsWork=buttonsWork&&std::abs(shown.values[i]-specified[slot].values[i])<.0002f;
        buttonsWork=buttonsWork&&factoryButtons[slot]->getToggleState()&&!droneEditor->presetIsModified();
    }
    waveCheck(buttonsWork,"Each archetype button loads the exact specified sound without a false MODIFIED status");
    setPlain(guiProcessor,"madness",.151f);
    droneEditor->refreshPresetSelection();
    waveCheck(droneEditor->presetIsModified(),"A manual sound edit marks the selected archetype MODIFIED");
    factoryButtons[2]->onClick();
    juce::Slider *madnessKnob=nullptr,*bassFader=nullptr;
    juce::Button *sawPad=nullptr,*squarePad=nullptr;
    for(auto* child:editor->getChildren())
    {
        if(child->getName()=="Encoder 8")madnessKnob=dynamic_cast<juce::Slider*>(child);
        if(child->getName()=="Fader 1")bassFader=dynamic_cast<juce::Slider*>(child);
        if(child->getName()=="Pad 4")sawPad=dynamic_cast<juce::Button*>(child);
        if(child->getName()=="Pad 5")squarePad=dynamic_cast<juce::Button*>(child);
    }
    waveCheck(madnessKnob&&bassFader&&sawPad&&squarePad
        &&madnessKnob->getTextFromValue(madnessKnob->getValue())=="66.6%"
        &&bassFader->getTextFromValue(bassFader->getValue())=="-7.5 dB"
        &&sawPad->getToggleState()&&squarePad->getToggleState(),
        "Inquisitor GUI shows exact decimal controls and its Square/Sine waveforms");
}
