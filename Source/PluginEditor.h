// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class DroneLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    juce::Font getTextButtonFont(juce::TextButton&, int) override;
    juce::Font getLabelFont(juce::Label&) override;
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
};
class WavePad final : public juce::Button
{
public:
    explicit WavePad(int number);
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
    static juce::Rectangle<float> keyBounds(int note);
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
class DroneEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit DroneEditor(DroneProcessor&);
    ~DroneEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    static juce::String footerQuote()
    {
        return juce::String::fromUTF8(u8"Никто не обольщай самого себя. Если кто из вас думает быть мудрым в веке сем, тот будь безумным, чтобы быть мудрым");
    }
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
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 12> attachments;
    juce::TextButton panic { "PANIC / SILENCE" };
    juce::TooltipWindow tooltips { this, 650 };
    float meter = 0;
    void timerCallback() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DroneEditor)
};
