// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class DroneLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    DroneLookAndFeel();
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
    juce::Font getTextButtonFont(juce::TextButton&, int) override;
    juce::Font getLabelFont(juce::Label&) override;
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
};
class WavePad final : public juce::Button
{
public:
    explicit WavePad(int number);
    static juce::Path waveformPath(int shape,juce::Rectangle<float> area);
    void paintButton(juce::Graphics&, bool highlighted, bool down) override;
private:
    int padNumber;
};
class PlayableKeyboard final : public juce::Component, public juce::SettableTooltipClient, private juce::Timer
{
public:
    explicit PlayableKeyboard(DroneProcessor&);
    ~PlayableKeyboard() override;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void focusLost(FocusChangeType) override;
    void focusGained(FocusChangeType) override { tracksKeyboardFocus = true; }
    void visibilityChanged() override;
    void parentHierarchyChanged() override;
    int noteAt(juce::Point<float>) const;
    static juce::Rectangle<float> keyBounds(int note,int first=48);
    int firstNote() const {return processor.keyboardFirstNote.load();}
    bool shiftOctave(int direction);
    void validateRange();
    void pressAt(juce::Point<float>);
    void dragAt(juce::Point<float>);
    void releaseNote();
    void checkPointerSafety(bool leftButtonDown, bool ownsFocus);
private:
    DroneProcessor& processor;
    int held = -1, hover = -1;
    bool tracksKeyboardFocus = false;
    unsigned epoch = 0;
    void switchNote(int note);
    void timerCallback() override;
};
class DroneEditor final : public juce::AudioProcessorEditor, private juce::Timer, private juce::KeyListener
{
public:
    explicit DroneEditor(DroneProcessor&);
    ~DroneEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool presetIsModified() const { return presetModified; }
    void refreshPresetSelection();
    bool computerKeyboardEnabled() const {return processor.acceptsComputerKeyboard();}
    void focusGained(FocusChangeType) override { updateComputerKeyboard(); }
    void focusLost(FocusChangeType) override { updateComputerKeyboard(); }
    void focusOfChildComponentChanged(FocusChangeType) override { updateComputerKeyboard(); }
    void visibilityChanged() override { updateComputerKeyboard(); }
    static juce::String footerQuote()
    {
        return "Let no man deceive himself. If any man among you seemeth to be wise in this world, let him become a fool, that he may be wise.";
    }
    static juce::String footerAttribution() { return "1 CORINTHIANS 3:18 - KJV"; }
private:
    DroneProcessor& processor;
    DroneLookAndFeel look;
    juce::Image panelBackdrop;
    PlayableKeyboard keyboard;
    std::array<juce::Slider, 8> knobs;
    std::array<juce::Slider, 4> faders;
    std::array<std::unique_ptr<WavePad>, 8> pads;
    juce::Slider pitchWheel, modWheel;
    juce::Slider polyphony;
    bool pitchDragging = false;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 13> attachments;
    juce::TextButton panic { "SILENCE" };
    juce::TextButton inspiration { "PRAY FOR INSPIRATION" };
    std::array<juce::TextButton,4> factoryPresets {
        juce::TextButton("THE HERMIT"), juce::TextButton("THE MARTYR"),
        juce::TextButton("THE INQUISITOR"), juce::TextButton("THE HIEROPHANT") };
    juce::TextButton customPreset { "THE FOOL (CUSTOM)" }, saveCustom { "SAVE" };
    yuro::CustomPresetStore customStore;
    yuro::SoundPreset selectedSound;
    int selectedPreset=-1;
    bool presetModified=false;
    juce::String presetStatus;
    juce::TextButton octaveDown { "<" },octaveUp { ">" };
    int inspirationFlash=0;
    juce::TooltipWindow tooltips { this, 650 };
    float meter = 0;
    ComputerKeyboard computerKeyboard;
    unsigned computerEpoch=0;
    bool keyPressed(const juce::KeyPress&,juce::Component*) override;
    bool keyStateChanged(bool,juce::Component*) override;
    void updateComputerKeyboard();
    void cancelComputerKeyboard();
    bool computerFocusAllowed() const;
    void timerCallback() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DroneEditor)
};
