// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "WaveformTests.h"
#include "../Source/PluginEditor.h"

inline void runVersion11Tests(const juce::File& root,const juce::String& pluginPath)
{
    const std::array<yuro::SoundPreset,4> expected {{
        {{.184f,.3162f,4,8,.541f,.15f,.33f,.08f,0,0,0,0,1}},
        {{.054f,.3605f,1.011f,13.905f,.237f,.498f,1,1,-.6f,.3f,6.6f,1,0}},
        {{1,.2045f,.005f,1.115f,.35f,0,1,.666f,-7.5f,0,2.9f,3,0}},
        {{.80f,.50f,3,15,.90f,.35f,1,.15f,-1,2,3,1,0}}
    }};
    auto matches=[&](DroneProcessor& p,size_t slot)
    {
        const auto sound=p.captureSound();bool same=true;
        for(size_t i=0;i<sound.values.size();++i)same=same&&std::abs(sound.values[i]-expected[slot].values[i])<.0002f;
        return same;
    };
    DroneProcessor fresh;
    waveCheck(matches(fresh,0)&&fresh.polyphonyLimit()==8&&std::abs(fresh.parameters.getRawParameterValue("master")->load()-.65f)<.0001f,
        "Fresh instrument starts with exact Hermit sound, eight voices and unchanged Master");
    for(size_t slot=0;slot<4;++slot)
    {
        setPlain(fresh,"master",.4321f);setPlain(fresh,"polyphony",12);fresh.setPitchFromMidi(12000);fresh.setModulation(87);
        fresh.applySoundPreset(yuro::archetypes[slot]);
        waveCheck(matches(fresh,slot)&&fresh.polyphonyLimit()==12&&std::abs(fresh.parameters.getRawParameterValue("master")->load()-.4321f)<.0001f
            &&fresh.modulation.load()==87&&fresh.pitchValue.load()==12000,"Calibrated archetype values/waves are exact and preserve Master, polyphony and expression");
    }
    DroneProcessor initial;
    {
        std::unique_ptr<DroneEditor> editor(static_cast<DroneEditor*>(initial.createEditor()));bool active=false;
        for(auto* child:editor->getChildren())if(auto* b=dynamic_cast<juce::TextButton*>(child))if(b->getButtonText()=="THE HERMIT")active=b->getToggleState();
        waveCheck(active&&!editor->presetIsModified(),"Fresh Hermit is selected in the editor without MODIFIED");
        juce::Image image=editor->createComponentSnapshot(editor->getLocalBounds());
        auto output=root.getChildFile("YURODSTVO-1.1-interface.png").createOutputStream();juce::PNGImageFormat().writeImageToStream(image,*output);
        waveCheck(image.getPixelAt(40,130).getRed()>image.getPixelAt(40,130).getGreen(),"Compact monitor uses a warm brown rather than a dominant green");
        for(auto* child:editor->getChildren())if(child->getName()=="Encoder 3")
            waveCheck(dynamic_cast<juce::Slider*>(child)->getTooltip()=="Attack. MiniLab CC 76 / DAW CC 89.","Attack tooltip uses verified User and DAW CC maps");
    }
    setPlain(initial,"mix",.8f);setPlain(initial,"shimmer",.7f);setPlain(initial,"osc2Waveform",3);setPlain(initial,"master",.3f);
    juce::MemoryBlock saved;initial.getStateInformation(saved);DroneProcessor restored;
    restored.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
    for(int reopen=0;reopen<2;++reopen){std::unique_ptr<juce::AudioProcessorEditor> editor(restored.createEditor());}
    waveCheck(std::abs(restored.parameters.getRawParameterValue("mix")->load()-.8f)<.0001f&&restored.currentWaveform(1)==3
        &&std::abs(restored.parameters.getRawParameterValue("master")->load()-.3f)<.0001f,
        "Host state overrides Hermit defaults and survives repeated editor opening");
    ComputerKeyboard keys;ComputerKeyboard::Keys down {};
    keys.update(down,48,true);down[0]=down[4]=down[7]=true;
    waveCheck(keys.update(down,48,true)[48]&&keys.notes()[52]&&keys.notes()[55],"A/D/G computer chord maps to MIDI 48/52/55");
    waveCheck(keys.update(down,48,true)[48],"Repeated key-down state retains one held target");
    down[0]=false;waveCheck(!keys.update(down,48,true)[48]&&keys.notes()[52],"Computer key-up releases only its own target");
    keys.update(down,60,true);waveCheck(!keys.notes()[52]&&!keys.notes()[64],"Octave change releases held notes and inhibits them until key-up");
    down.fill(false);keys.update(down,60,true);down[12]=true;waveCheck(keys.update(down,60,true)[72],"K plays the upper C relative to the visible octave");
    keys.update(down,60,false);waveCheck(!keys.notes()[72],"Focus/text-entry cancellation releases computer notes");
    keys.update(down,60,true);waveCheck(!keys.notes()[72],"Returning focus with a held key cannot resurrect a note");
    down.fill(false);keys.update(down,60,true);down[12]=true;waveCheck(keys.update(down,60,true)[72],"Releasing and pressing after cancellation permits a new note");
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Standalone);
    DroneProcessor standalone;
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
    waveCheck(standalone.acceptsComputerKeyboard()&&matches(standalone,0),"Standalone wrapper enables computer input and starts with Hermit");
    neutralTestSound(standalone);setPlain(standalone,"attack",.005f);setPlain(standalone,"release",.1f);standalone.prepareToPlay(48000,256);
    ComputerKeyboard::Notes target {};target[48]=target[52]=target[55]=true;standalone.setComputerNotes(target);
    juce::AudioBuffer<float> audio(2,256);juce::MidiBuffer none;standalone.processBlock(audio,none);
    waveCheck(standalone.voices.load()==3&&standalone.notes[48].load(),"Computer chord enters the shared engine and visible polyphony/note state");
    for(int i=0;i<8;++i){standalone.setComputerNotes(target);juce::MidiBuffer m;standalone.processBlock(audio,m);}
    waveCheck(standalone.voices.load()==3,"Computer repeat events cannot duplicate Note On voices");
    juce::MidiBuffer midi;midi.addEvent(juce::MidiMessage::noteOn(1,48,.8f),0);standalone.setMouseNote(48);standalone.processBlock(audio,midi);
    waveCheck(standalone.voices.load()==5,"Physical MIDI, mouse and computer can independently own the same pitch");
    target.fill(false);standalone.setComputerNotes(target);
    for(int i=0;i<30;++i){juce::MidiBuffer m;standalone.processBlock(audio,m);}
    waveCheck(standalone.voices.load()==2&&standalone.noteLevels[48].load()>0,"Computer Note Off and Release preserve mouse/MIDI ownership and glow");
    standalone.setMouseNote(-1);midi.addEvent(juce::MidiMessage::noteOff(1,48),0);standalone.processBlock(audio,midi);
    for(int i=0;i<30;++i){juce::MidiBuffer m;standalone.processBlock(audio,m);}
    waveCheck(standalone.voices.load()==0,"All input owners release without stuck voices");
    target[36]=true;standalone.setComputerNotes(target);standalone.processBlock(audio,none);
    waveCheck(standalone.voices.load()==0,"Computer targets cannot play the reserved waveform pad octave");
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_VST3);DroneProcessor plugin;
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
    plugin.prepareToPlay(48000,256);target[48]=true;plugin.setComputerNotes(target);plugin.processBlock(audio,none);
    std::unique_ptr<DroneEditor> pluginEditor(static_cast<DroneEditor*>(plugin.createEditor()));
    waveCheck(!pluginEditor->computerKeyboardEnabled()&&plugin.voices.load()==0,"VST3 wrapper rejects computer notes and installs no computer key capture");
    if(pluginPath.isNotEmpty())
    {
        juce::VST3PluginFormat format;juce::OwnedArray<juce::PluginDescription> descriptions;
        format.findAllTypesForFile(descriptions,pluginPath);waveCheck(!descriptions.isEmpty(),"New VST3 can be scanned for startup checks");
        juce::String error;auto hosted=format.createInstanceFromDescription(*descriptions[0],48000,256,error);
        waveCheck(hosted!=nullptr,"Actual VST3 instance loads for Hermit startup check");
        DroneProcessor defaults;bool same=true;int found=0;
        for(const auto* id:yuro::soundIds)
            for(auto* parameter:hosted->getParameters())if(parameter->getName(80)==defaults.parameters.getParameter(id)->getName(80))
            {++found;same=same&&std::abs(parameter->getValue()-defaults.parameters.getParameter(id)->getValue())<.0001f;}
        waveCheck(same&&found==13,"Actual fresh VST3 exposes all thirteen Hermit startup settings");
        // VST3 host blobs include wrapper/controller state; do not feed raw APVTS XML
        // directly to a hosted instance. Save through the actual VST3 wrapper.
        for(const auto* id:yuro::soundIds)
            for(auto* parameter:hosted->getParameters())if(parameter->getName(80)==restored.parameters.getParameter(id)->getName(80))
                parameter->setValueNotifyingHost(restored.parameters.getParameter(id)->getValue());
        juce::MemoryBlock hostState;hosted->getStateInformation(hostState);
        auto reopened=format.createInstanceFromDescription(*descriptions[0],48000,256,error);
        waveCheck(reopened!=nullptr,"Second actual VST3 instance loads for saved-state restoration");
        reopened->setStateInformation(hostState.getData(),static_cast<int>(hostState.getSize()));same=true;found=0;
        for(const auto* id:yuro::soundIds)
            for(auto* parameter:reopened->getParameters())if(parameter->getName(80)==restored.parameters.getParameter(id)->getName(80))
            {++found;same=same&&std::abs(parameter->getValue()-restored.parameters.getParameter(id)->getValue())<.0001f;}
        waveCheck(same&&found==13,"Actual VST3 host-state restore replaces startup Hermit without resetting the saved sound");
    }
}
