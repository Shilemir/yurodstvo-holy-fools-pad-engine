// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include "WaveformTests.h"
#include "../Source/PluginEditor.h"
#include <BinaryData.h>
#include <thread>

inline void runKeyboardTests()
{
    const auto image=juce::ImageCache::getFromMemory(YuroAssets::yurodstvo_icon_png,YuroAssets::yurodstvo_icon_pngSize);
    waveCheck(image.isValid()&&image.getWidth()==1536&&image.getHeight()==1024,"Official PNG decodes entirely from embedded plugin resources");
    int oldBytes=0;
    waveCheck(YuroAssets::getNamedResource("tye_andrey_jpg",oldBytes)==nullptr,"Previous icon is no longer part of BinaryData");
    DroneProcessor p;setPlain(p,"attack",0.005f);setPlain(p,"release",0.005f);p.prepareToPlay(48000,256);
    juce::AudioBuffer<float> audio(2,256);
    auto process=[&](int blocks=1)
    {
        for(int i=0;i<blocks;++i){juce::MidiBuffer m;p.processBlock(audio,m);}
    };
    auto send=[&](juce::MidiMessage message)
    {juce::MidiBuffer m;m.addEvent(message,0);p.processBlock(audio,m);};
    auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());PlayableKeyboard* keys=nullptr;
    for(auto* child:editor->getChildren())if(child->getName()=="On-screen keyboard")keys=dynamic_cast<PlayableKeyboard*>(child);
    waveCheck(keys&&keys->getBounds()==juce::Rectangle<int>(210,613,945,80)&&keys->firstNote()==48,"Interactive keyboard keeps its width and defaults to MIDI C2");
    auto event=[&](juce::Point<float> point,int flags,bool dragged=false)
    {
        const auto now=juce::Time::getCurrentTime();
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,juce::ModifierKeys(flags),
            1,0,0,0,0,keys,keys,now,point,now,1,dragged);
    };
    bool hits=true;
    for(int note=48;note<=72;++note)hits=hits&&keys->noteAt(PlayableKeyboard::keyBounds(note,48).getCentre())==note;
    waveCheck(hits&&keys->noteAt({-1,20})==-1&&keys->noteAt({950,20})==-1,"All white/black keys hit correctly with black keys above whites and no out-of-range notes");
    const auto c=PlayableKeyboard::keyBounds(60,48).getCentre(),d=PlayableKeyboard::keyBounds(62,48).getCentre();
    keys->mouseDown(event(c,juce::ModifierKeys::rightButtonModifier));process();
    waveCheck(p.voices.load()==0,"Right mouse button does not start a note");
    keys->mouseDown(event(c,juce::ModifierKeys::leftButtonModifier));process(20);
    waveCheck(p.voices.load()==1&&p.notes[60].load()&&audio.getMagnitude(0,256)>0.001f,"Left mouse down produces audio and highlights its sounding voice");
    for(int i=0;i<20;++i)keys->mouseDrag(event(c,juce::ModifierKeys::leftButtonModifier,true));process();
    waveCheck(p.voices.load()==1,"Holding or dragging within the same key does not duplicate Note On");
    keys->mouseDrag(event(d,juce::ModifierKeys::leftButtonModifier,true));process(10);
    waveCheck(p.voices.load()==1&&!p.notes[60].load()&&p.notes[62].load(),"Dragging to another key releases the previous note and sounds the new one");
    keys->mouseUp(event(d,0));process(10);
    waveCheck(p.voices.load()==0&&audio.getMagnitude(0,256)==0,"Mouse up releases the voice through the normal envelope");
    keys->mouseDown(event(c,juce::ModifierKeys::leftButtonModifier));process();
    keys->mouseDrag(event({-5,90},juce::ModifierKeys::leftButtonModifier,true));process(10);
    waveCheck(p.voices.load()==0,"Dragging outside the keyboard releases the note even while mouse capture is held");
    keys->pressAt(c);process();keys->mouseExit(event({-1,20},juce::ModifierKeys::leftButtonModifier));process(10);
    waveCheck(p.voices.load()==0,"Mouse exit cannot leave a held voice");
    keys->pressAt(c);process();keys->focusLost(juce::Component::focusChangedDirectly);process(10);
    waveCheck(p.voices.load()==0,"Focus loss sends a release");
    keys->pressAt(c);process();keys->checkPointerSafety(false,true);process(10);
    waveCheck(p.voices.load()==0,"Safety poll releases a note if the host missed Mouse Up");
    keys->pressAt(c);process();keys->checkPointerSafety(true,false);process(10);
    waveCheck(p.voices.load()==0,"Safety poll releases on deactivation even with the button down");
    keys->pressAt(c);process();keys->visibilityChanged();process(10);
    waveCheck(p.voices.load()==0,"Hiding the editor releases a mouse note");
    keys->pressAt(c);send(juce::MidiMessage::noteOn(1,60,0.8f));process(10);
    waveCheck(p.voices.load()==2,"Mouse and physical MIDI can independently hold the same note on channel one");
    keys->releaseNote();process(10);
    waveCheck(p.voices.load()==1&&p.notes[60].load(),"Mouse release does not cut a matching physical MIDI note");
    send(juce::MidiMessage::noteOff(1,60));process(10);
    keys->pressAt(c);send(juce::MidiMessage::noteOn(1,60,0.8f));process();
    send(juce::MidiMessage::noteOff(1,60));process(10);
    waveCheck(p.voices.load()==1&&p.notes[60].load(),"Physical Note Off does not cut a matching mouse note");
    keys->releaseNote();process(10);
    send(juce::MidiMessage::controllerEvent(1,64,127));keys->pressAt(c);send(juce::MidiMessage::noteOn(1,60,0.8f));
    send(juce::MidiMessage::noteOff(1,60));keys->releaseNote();process(10);
    waveCheck(p.voices.load()==1,"Physical sustain still holds MIDI while mouse release remains reliable");
    send(juce::MidiMessage::controllerEvent(1,64,0));process(10);
    waveCheck(p.voices.load()==0,"Sustain release leaves no stuck notes from either source");
    keys->pressAt(c);process();juce::MemoryBlock saved;p.getStateInformation(saved);
    DroneProcessor restored;restored.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));restored.prepareToPlay(48000,256);
    {juce::MidiBuffer m;restored.processBlock(audio,m);}
    waveCheck(restored.voices.load()==0,"Held mouse gesture is transient and is not restored from a preset");
    p.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));process(10);keys->checkPointerSafety(true,true);process();
    waveCheck(p.voices.load()==0,"Loading a state cancels an already active mouse gesture");
    p.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));keys->pressAt(c);process();
    keys->checkPointerSafety(true,true);process();
    waveCheck(p.voices.load()==1,"A new click after state loading is not discarded by the previous gesture's cancellation");
    keys->releaseNote();process(10);
    keys->pressAt(c);process();editor.reset();process(10);
    waveCheck(p.voices.load()==0,"Closing the editor releases only its mouse ownership");
    p.setMouseNote(60);process();
    for(int i=0;i<2000;++i)p.setMouseNote(60);
    p.setMouseNote(-1);process(10);
    waveCheck(p.voices.load()==0,"A full bounded mouse queue cannot lose the final Note Off");
    p.setMouseNote(60);process();for(int i=0;i<2000;++i)p.setMouseNote(60);
    p.panicRequested.store(true);process(20);
    waveCheck(p.voices.load()==0,"Panic discards stale queued mouse notes without restarting them");
    p.setMouseNote(60);process();waveCheck(p.voices.load()==1,"A new mouse gesture works after Panic");
    p.setMouseNote(-1);process(10);
    setPlain(p,"polyphony",1);p.setMouseNote(60);process();send(juce::MidiMessage::noteOn(1,60,0.8f));process(10);
    p.setMouseNote(-1);process(10);
    waveCheck(p.voices.load()==1,"Mouse release after voice stealing does not stop the replacement MIDI voice");
    send(juce::MidiMessage::noteOff(1,60));process(10);
    send(juce::MidiMessage::noteOn(1,60,0.8f));p.setMouseNote(60);process(10);
    send(juce::MidiMessage::noteOff(1,60));process(10);
    waveCheck(p.voices.load()==1,"MIDI release after voice stealing does not stop the replacement mouse voice");
    p.setMouseNote(-1);process(10);
    setPlain(p,"polyphony",8);setPlain(p,"release",0.3f);p.setMouseNote(60);process(20);p.setMouseNote(-1);process();
    waveCheck(p.voices.load()==1&&audio.getMagnitude(0,256)>0,"Mouse Note Off starts the existing Release instead of cutting audio");
    process(80);waveCheck(p.voices.load()==0,"Mouse Release finishes and frees its voice");
    setPlain(p,"release",0.005f);p.setMouseNote(60);process();p.releaseResources();process(10);
    waveCheck(p.voices.load()==0,"Audio shutdown cancels the temporary mouse source");
    setPlain(p,"padInput",0);setPlain(p,"padFirstNote",48);setPlain(p,"padChannel",1);p.setMouseNote(48);process(10);
    waveCheck(p.voices.load()==0,"Virtual keys respect the reserved pad octave even with a saved custom pad map");
    send(juce::MidiMessage::noteOn(1,50,1.0f));process();
    waveCheck(p.voices.load()==0&&p.currentWaveform(0)==2,"Physical pad mapping still selects waveforms without starting a voice");
    p.setMouseNote(-1);process(10);
    yuro::DroneEngine mouse, midi;mouse.prepare(48000);midi.prepare(48000);
    mouse.setTargets({0,0.5f,0.005f,1,1});midi.setTargets({0,0.5f,0.005f,1,1});
    mouse.noteOn(1,60,0.8f,1);midi.noteOn(1,60,0.8f);bool same=true;
    for(int i=0;i<10000;++i)same=same&&mouse.nextSample()==midi.nextSample();
    waveCheck(same,"Mouse and MIDI use exactly the same oscillator/envelope engine at velocity 0.8");
    MouseNoteInput queue;std::atomic<bool> done{false};int last=-2;bool valid=true;
    std::thread producer([&]{for(int i=0;i<50000;++i)queue.post(48+i%25);queue.post(-1);done.store(true,std::memory_order_release);});
    auto receive=[&](int note){last=note;valid=valid&&(note==-1||(note>=48&&note<=72));};
    while(!done.load(std::memory_order_acquire)){queue.drain(receive);std::this_thread::yield();}
    producer.join();queue.drain(receive);
    waveCheck(valid&&last==-1,"Concurrent producer/audio-consumer mouse requests preserve the final release under overflow");
}
