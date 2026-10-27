// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#include "PluginEditor.h"
#include <BinaryData.h>

namespace
{
const juce::Colour background(0xff3a372b), border(0xff80724e), text(0xffeee1c4), accent(0xffd2b875), muted(0xffc2b597);
constexpr std::array<int, 4> knobX { 250, 384, 518, 652 };
juce::Font sacredFont(float height)
{
    static const auto face = juce::Typeface::createSystemTypefaceFor(
        YuroAssets::RuslanDisplayRegular_ttf, YuroAssets::RuslanDisplayRegular_ttfSize);
    return juce::Font(juce::FontOptions(face).withHeight(height));
}
void label(juce::Graphics& g, juce::String s, int x, int y, int w, int h, float size, juce::Colour colour,
           juce::Justification align = juce::Justification::centredLeft, bool numeric = false)
{
    g.setColour(colour);
    g.setFont(numeric ? juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain)) : sacredFont(size));
    g.drawText(s, x, y, w, h, align);
}
void readout(juce::Graphics& g, juce::String s, int x, int y, int w, int h, float size, juce::Colour colour,
             juce::Justification align = juce::Justification::centredLeft)
{
    label(g, s, x, y, w, h, size, colour, align, true);
}
void diamond(juce::Graphics& g, float x, float y, float radius)
{
    juce::Path p;
    p.startNewSubPath(x, y - radius); p.lineTo(x + radius, y);
    p.lineTo(x, y + radius); p.lineTo(x - radius, y); p.closeSubPath();
    g.strokePath(p, juce::PathStrokeType(1));
}
juce::Image makeBackdrop()
{
    // Built once and shared read-only by editors; paint never rescales the PNG.
    static const juce::Image cached = []
    {
        juce::Image result(juce::Image::ARGB, 1160, 470, true);
        juce::Graphics bg(result);
        bg.setGradientFill(juce::ColourGradient(juce::Colour(0xff665734), 0, 0,
            juce::Colour(0xff41432c), 1160, 470, false));
        bg.fillAll();
        const auto icon = juce::ImageCache::getFromMemory(YuroAssets::yurodstvo_icon_png, YuroAssets::yurodstvo_icon_pngSize);
        if (!icon.isValid()) return result;
        constexpr float scale = 0.78f;
        const int w = juce::roundToInt(icon.getWidth()*scale), h = juce::roundToInt(icon.getHeight()*scale);
        // Source face: about (0.66w,0.285h). Target: panel-local (835,235).
        const float faceX=0.66f*w, faceY=0.285f*h;
        juce::Image layer(juce::Image::ARGB,w,h,true);
        { juce::Graphics image(layer); image.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
          image.drawImage(icon,layer.getBounds().toFloat(),juce::RectanglePlacement::centred); }
        { juce::Image::BitmapData pixels(layer,juce::Image::BitmapData::readWrite);
        for(int y=0;y<h;++y) for(int x=0;x<w;++x)
        {
            const float ex=juce::jlimit(0.0f,1.0f,static_cast<float>(juce::jmin(x,w-1-x))/140);
            const float ey=juce::jlimit(0.0f,1.0f,static_cast<float>(juce::jmin(y,h-1-y))/70);
            const float dx=(x-faceX)/135,dy=(y-faceY)/155;
            const float focus=std::exp(-0.5f*(dx*dx+dy*dy));
            const float alpha=(0.61f+0.29f*focus)*ex*ex*(3-2*ex)*ey*ey*(3-2*ey);
            const auto toned=pixels.getPixelColour(x,y).interpolatedWith(juce::Colour(0xff76623e),0.08f);
            pixels.setPixelColour(x,y,toned.withMultipliedAlpha(alpha));
        }
        }
        bg.drawImageAt(layer,juce::roundToInt(835-faceX),juce::roundToInt(235-faceY));
        // A soft edge vignette unifies the full panel, without covering the face.
        bg.setGradientFill(juce::ColourGradient(juce::Colours::transparentBlack,835,235,
            juce::Colour(0xff282c20).withAlpha(0.24f),1420,235,true)); bg.fillAll();
        return result;
    }();
    return cached;
}
bool isBlackKey(int note) { const int p=note%12; return p==1||p==3||p==6||p==8||p==10; }
}

PlayableKeyboard::PlayableKeyboard(DroneProcessor& p):processor(p),epoch(p.mouseEpoch.load())
{
    setName("On-screen keyboard"); setWantsKeyboardFocus(true);
    setTooltip("Drag to play C2-C4 / velocity 0.8. Release or leave the keyboard to release the note.");
    startTimerHz(30);
}
PlayableKeyboard::~PlayableKeyboard() { stopTimer(); releaseNote(); }
juce::Rectangle<float> PlayableKeyboard::keyBounds(int note)
{
    if(note<48||note>72) return {};
    int whites=0; for(int n=48;n<note;++n) if(!isBlackKey(n)) ++whites;
    return isBlackKey(note)?juce::Rectangle<float>(static_cast<float>(whites*63-18),0,33,46)
                           :juce::Rectangle<float>(static_cast<float>(whites*63),0,60,80);
}
int PlayableKeyboard::noteAt(juce::Point<float> point) const
{
    if(!getLocalBounds().toFloat().contains(point)) return -1;
    for(int note=48;note<=72;++note) if(isBlackKey(note)&&keyBounds(note).contains(point)) return note;
    for(int note=48;note<=72;++note) if(!isBlackKey(note))
    {
        auto hit=keyBounds(note); hit.setWidth(63);
        if(hit.contains(point)) return note;
    }
    return -1;
}
void PlayableKeyboard::switchNote(int note)
{
    if(note==held) return;
    held=note; processor.setMouseNote(note); repaint();
}
void PlayableKeyboard::pressAt(juce::Point<float> point)
{
    epoch=processor.mouseEpoch.load(); if(isShowing()) grabKeyboardFocus();
    tracksKeyboardFocus=hasKeyboardFocus(true);
    hover=noteAt(point); switchNote(hover);
}
void PlayableKeyboard::dragAt(juce::Point<float> point) { hover=noteAt(point); switchNote(hover); }
void PlayableKeyboard::releaseNote() { switchNote(-1); }
void PlayableKeyboard::mouseDown(const juce::MouseEvent& e) { if(e.mods.isLeftButtonDown()) pressAt(e.position); }
void PlayableKeyboard::mouseDrag(const juce::MouseEvent& e) { if(e.mods.isLeftButtonDown()) dragAt(e.position); else releaseNote(); }
void PlayableKeyboard::mouseUp(const juce::MouseEvent&) { releaseNote(); }
void PlayableKeyboard::mouseMove(const juce::MouseEvent& e) { hover=noteAt(e.position); repaint(); }
void PlayableKeyboard::mouseExit(const juce::MouseEvent&) { hover=-1; releaseNote(); repaint(); }
void PlayableKeyboard::focusLost(FocusChangeType) { releaseNote(); }
void PlayableKeyboard::visibilityChanged() { if(!isShowing()) releaseNote(); }
void PlayableKeyboard::parentHierarchyChanged() { if(getParentComponent()==nullptr) releaseNote(); }
void PlayableKeyboard::checkPointerSafety(bool leftButtonDown,bool ownsFocus)
{
    const auto current=processor.mouseEpoch.load();
    if(!leftButtonDown||!ownsFocus||current!=epoch) releaseNote();
    epoch=current;
}
void PlayableKeyboard::timerCallback()
{
    checkPointerSafety(juce::ModifierKeys::getCurrentModifiersRealtime().isLeftButtonDown(),
        isShowing()&&juce::Process::isForegroundProcess()&&(!tracksKeyboardFocus||hasKeyboardFocus(true)));
    repaint();
}
void PlayableKeyboard::paint(juce::Graphics& g)
{
    auto draw=[&](int note)
    {
        const auto bounds=keyBounds(note);
        const bool active=processor.notes[static_cast<size_t>(note)].load()||note==held;
        auto colour=isBlackKey(note)?background:juce::Colour(0xff50513a);
        if(active) colour=isBlackKey(note)?accent.darker():accent;
        else if(note==hover) colour=colour.interpolatedWith(accent,0.20f);
        g.setColour(colour); g.fillRoundedRectangle(bounds,2);
        if(note==held) {g.setColour(text.withAlpha(0.75f));g.drawRoundedRectangle(bounds.reduced(2),2,0.8f);}
        if(note%12==0) readout(g,juce::MidiMessage::getMidiNoteName(note,true,true,3),
            juce::roundToInt(bounds.getX())+5,56,50,20,10,active?background:text);
    };
    for(int n=48;n<=72;++n) if(!isBlackKey(n)) draw(n);
    for(int n=48;n<=72;++n) if(isBlackKey(n)) draw(n);
}
juce::Font DroneLookAndFeel::getTextButtonFont(juce::TextButton& button, int)
{
    if (const auto* parent = button.getParentComponent(); parent && parent->getName() == "Polyphony")
        return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 15, juce::Font::plain));
    return sacredFont(16);
}
juce::Font DroneLookAndFeel::getLabelFont(juce::Label& labelToDraw)
{
    const auto* parent = labelToDraw.getParentComponent();
    const bool eq = parent && parent->getName().startsWith("Fader") && parent->getName() != "Fader 1";
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), eq ? 10.5f : 13.0f, juce::Font::plain));
}
void DroneLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)).reduced(9);
    const auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
    const auto circle = juce::Rectangle<float>(diameter, diameter).withCentre(bounds.getCentre());
    const bool enabled = slider.isEnabled();
    g.setGradientFill(juce::ColourGradient(enabled ? juce::Colour(0xff5b573e) : juce::Colour(0xff444733), circle.getX(), circle.getY(),
        juce::Colour(0xff303425), circle.getRight(), circle.getBottom(), false));
    g.fillEllipse(circle); g.setColour(border); g.drawEllipse(circle, 1.5f);
    juce::Path arc; const float radius = diameter * 0.5f + 3;
    arc.addCentredArc(circle.getCentreX(), circle.getCentreY(), radius, radius, 0, start, end, true);
    g.strokePath(arc, juce::PathStrokeType(1));
    g.setColour(accent.withAlpha(enabled ? 0.22f : 0.07f)); g.drawEllipse(circle.reduced(4), 0.7f);
    if (enabled)
    {
        juce::Path progress; const float angle = start + pos * (end - start);
        progress.addCentredArc(circle.getCentreX(), circle.getCentreY(), radius, radius, 0, start, angle, true);
        g.setColour(accent); g.strokePath(progress, juce::PathStrokeType(2));
        const auto centre = circle.getCentre();
        g.drawLine(centre.x + std::sin(angle) * diameter * 0.23f, centre.y - std::cos(angle) * diameter * 0.23f,
                   centre.x + std::sin(angle) * diameter * 0.42f, centre.y - std::cos(angle) * diameter * 0.42f, 2.5f);
    }
}
void DroneLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& slider)
{
    const float cx = static_cast<float>(x) + static_cast<float>(w) * 0.5f;
    g.setColour(border); g.drawLine(cx, static_cast<float>(y), cx, static_cast<float>(y + h), 3);
    for (int i = 0; i <= 8; ++i)
    {
        const float yy = static_cast<float>(y) + static_cast<float>(h * i) / 8;
        g.drawLine(cx - 15, yy, cx - 8, yy, 1);
    }
    if (slider.isEnabled())
    {
        const float origin = slider.getMinimum() < 0 ? static_cast<float>(y) + static_cast<float>(h) * 0.5f : static_cast<float>(y + h);
        g.setColour(accent); g.drawLine(cx, pos, cx, origin, 3);
        if (slider.getMinimum() < 0) { g.setColour(muted); g.drawLine(cx - 17, origin, cx + 17, origin, 0.8f); }
        g.setColour(text); g.fillRoundedRectangle(cx - 13, pos - 6, 26, 12, 2);
        g.setColour(background); g.drawLine(cx - 9, pos, cx + 9, pos, 1);
    }
}

WavePad::WavePad(int number) : Button("Pad " + juce::String(number + 1)), padNumber(number)
{
    setButtonText("P" + juce::String(number + 1) + " / " + yuro::waveformNames[static_cast<size_t>(number % 4)]);
    setTooltip("Select oscillator " + juce::String(number / 4 + 1) + ": " + yuro::waveformNames[static_cast<size_t>(number % 4)]);
}
void WavePad::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    const bool selected = getToggleState();
    const auto bounds = getLocalBounds().toFloat().reduced(1);
    g.setColour(selected ? juce::Colour(0xff665b34) : background.withAlpha(0.93f)); g.fillRoundedRectangle(bounds, 4);
    if (highlighted || down) { g.setColour(accent.withAlpha(down ? 0.16f : 0.08f)); g.fillRoundedRectangle(bounds, 4); }
    g.setColour(selected ? accent : border); g.drawRoundedRectangle(bounds, 4, selected ? 1.7f : 1);
    readout(g, "P" + juce::String(padNumber + 1), 8, 4, 25, 17, 11, selected ? ::text : muted);
    label(g, juce::String(yuro::waveformNames[static_cast<size_t>(padNumber % 4)]).toUpperCase(), 7, 21, getWidth() - 14, 18, 13, ::text, juce::Justification::centred);
    juce::Path wave;
    for (int i = 0; i <= 80; ++i)
    {
        const float phase = static_cast<float>(i) / 40;
        const float p = phase - std::floor(phase);
        float value = 0;
        switch (padNumber % 4)
        {
            case 0: value = std::sin(phase * juce::MathConstants<float>::twoPi); break;
            case 1: value = (2 / juce::MathConstants<float>::pi) * std::asin(std::sin(phase * juce::MathConstants<float>::twoPi)); break;
            case 2: value = 2 * p - 1; break;
            case 3: value = p < 0.5f ? 0.85f : -0.85f; break;
        }
        const float x = 10 + static_cast<float>(getWidth() - 20) * i / 80;
        const float y = 53 - value * 9;
        if (i == 0) wave.startNewSubPath(x, y); else wave.lineTo(x, y);
    }
    g.setColour(selected ? accent.brighter(0.25f) : muted); g.strokePath(wave, juce::PathStrokeType(1.4f));
    if (selected) { g.setColour(accent); g.fillEllipse(static_cast<float>(getWidth() - 14), 8, 4, 4); }
}

DroneEditor::DroneEditor(DroneProcessor& p) : AudioProcessorEditor(p), processor(p), panelBackdrop(makeBackdrop()), keyboard(p)
{
    setLookAndFeel(&look); addAndMakeVisible(keyboard);
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& k = knobs[i];
        k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        k.setTextBoxStyle(i < 6 ? juce::Slider::TextBoxBelow : juce::Slider::NoTextBox, false, 100, 22);
        k.setColour(juce::Slider::textBoxTextColourId, text); k.setColour(juce::Slider::textBoxBackgroundColourId, background);
        k.setColour(juce::Slider::textBoxOutlineColourId, border); k.setEnabled(i < 6);
        k.setName("Encoder " + juce::String(static_cast<int>(i + 1)));
        const int controller = i < 4 ? yuro::midiCC[i] : (i == 4 ? 93 : 18);
        k.setTooltip(i < 6 ? "MiniLab 3: User CC " + juce::String(controller) + " / DAW CC "
            + juce::String(yuro::dawControlCC[i]) + ". Absolute 0-127. Drag or enter a value." : "Reserved for a future version");
        addAndMakeVisible(k);
        if (i < 6)
        {
            const auto attachment = i < 4 ? i : i + 1;
            const char* id = i == 0 ? "shimmer" : i < 4 ? yuro::parameterIds[i] : (i == 4 ? "reverb" : "delay");
            attachments[attachment] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, id, k);
            k.setNumDecimalPlacesToDisplay(3);
            k.setTextValueSuffix(i == 2 || i == 3 ? " s" : "");
            if (i == 0)
            {
                k.textFromValueFunction = [](double v) { return juce::String(v * 100, 0) + "%"; };
                k.valueFromTextFunction = [](const juce::String& s) { return s.getDoubleValue() / 100; };
                k.updateText();
            }
        }
    }
    for (size_t i = 0; i < faders.size(); ++i)
    {
        auto& f = faders[i]; f.setSliderStyle(juce::Slider::LinearVertical);
        f.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 22);
        f.setColour(juce::Slider::textBoxTextColourId, text); f.setColour(juce::Slider::textBoxBackgroundColourId, background);
        f.setColour(juce::Slider::textBoxOutlineColourId, border); f.setEnabled(true);
        f.setName("Fader " + juce::String(static_cast<int>(i + 1)));
        f.setTooltip(i == 0 ? "Master Volume / User CC 82 / DAW CC 14" : "Pre-FX EQ / +/-12 dB / double-click for 0 dB"); addAndMakeVisible(f);
    }
    attachments[4] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "master", faders[0]);
    faders[0].setNumDecimalPlacesToDisplay(3);
    constexpr std::array<const char*, 3> eqIds { "eqBass", "eqMid", "eqTreble" };
    for (size_t i = 0; i < 3; ++i)
    {
        attachments[i + 7] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, eqIds[i], faders[i + 1]);
        auto& fader = faders[i + 1]; fader.setDoubleClickReturnValue(true, 0);
        fader.textFromValueFunction = [](double value) { return (value > 0.049 ? "+" : "") + juce::String(std::abs(value) < 0.049 ? 0.0 : value, 1) + " dB"; };
        fader.valueFromTextFunction = [](const juce::String& value) { return value.getDoubleValue(); };
        fader.updateText();
    }
    for (size_t i = 0; i < pads.size(); ++i)
    {
        pads[i] = std::make_unique<WavePad>(static_cast<int>(i));
        pads[i]->onClick = [this, i] { processor.selectWaveform(static_cast<int>(i / 4), static_cast<int>(i % 4)); timerCallback(); };
        addAndMakeVisible(*pads[i]);
    }
    pitchWheel.setName("Pitch Wheel"); modWheel.setName("Mod Wheel");
    for (auto* wheel : { &pitchWheel, &modWheel })
    {
        wheel->setSliderStyle(juce::Slider::LinearVertical); wheel->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        wheel->setScrollWheelEnabled(false); addAndMakeVisible(*wheel);
    }
    pitchWheel.setRange(-100, 100, 0.001); pitchWheel.setDoubleClickReturnValue(true, 0);
    pitchWheel.setTooltip("Pitch Bend: oscillator II detune +/-100 cents. Release to centre. Legacy Detune automation remains compatible.");
    pitchWheel.onValueChange = [this] { processor.setPitchFromMouse(static_cast<float>(pitchWheel.getValue())); };
    pitchWheel.onDragStart = [this] { pitchDragging = true; };
    pitchWheel.onDragEnd = [this] { pitchDragging = false; processor.setPitchFromMouse(0); pitchWheel.setValue(0, juce::dontSendNotification); };
    modWheel.setRange(0, 127, 1); modWheel.setTooltip("CC1 Modulation / Reserved: no audio destination in version 0.6.");
    modWheel.onValueChange = [this] { processor.setModulation(juce::roundToInt(modWheel.getValue())); };
    panic.onClick = [this] { processor.panicRequested.store(true); };
    panic.setColour(juce::TextButton::buttonColourId, background); panic.setColour(juce::TextButton::textColourOffId, accent);
    polyphony.setName("Polyphony"); polyphony.setSliderStyle(juce::Slider::IncDecButtons);
    polyphony.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 65, 27);
    polyphony.setTextValueSuffix(" / 16"); polyphony.setDoubleClickReturnValue(true, 8);
    polyphony.setTooltip("Maximum simultaneous voices: 1-16. Default 8. Excess voices fade over 10 ms.");
    polyphony.setColour(juce::Slider::textBoxTextColourId, text); polyphony.setColour(juce::Slider::textBoxBackgroundColourId, background);
    polyphony.setColour(juce::Slider::textBoxOutlineColourId, border); addAndMakeVisible(polyphony);
    attachments[10] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "polyphony", polyphony);
    addAndMakeVisible(panic);
    setSize(1200, 780); timerCallback(); startTimerHz(30);
}
DroneEditor::~DroneEditor() { stopTimer(); setLookAndFeel(nullptr); }
void DroneEditor::resized()
{
    for (size_t i = 0; i < knobs.size(); ++i)
        knobs[i].setBounds(knobX[i % 4], 156 + static_cast<int>(i / 4) * 164, 112, i < 6 ? 107 : 85);
    for (size_t i = 0; i < faders.size(); ++i) faders[i].setBounds(956 + static_cast<int>(i) * 56, 171, 54, 239);
    for (size_t i = 0; i < pads.size(); ++i)
        pads[i]->setBounds((i < 4 ? 250 : 700) + static_cast<int>(i % 4) * 106, 468, 96, 72);
    panic.setBounds(38, 498, 186, 32); keyboard.setBounds(210,613,945,80);
    polyphony.setBounds(53, 437, 156, 29);
    pitchWheel.setBounds(42, 613, 35, 80); modWheel.setBounds(116, 613, 35, 80);
}
void DroneEditor::timerCallback()
{
    meter = juce::jmax(processor.peak.load(), meter * 0.90f);
    for (size_t i = 0; i < pads.size(); ++i)
        pads[i]->setToggleState(processor.currentWaveform(static_cast<int>(i / 4)) == static_cast<int>(i % 4), juce::dontSendNotification);
    if (!pitchDragging) pitchWheel.setValue(processor.currentDetune(), juce::dontSendNotification);
    modWheel.setValue(processor.modulation.load(), juce::dontSendNotification);
    repaint();
}
void DroneEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);
    g.drawImageAt(panelBackdrop, 20, 98);
    g.setColour(border); g.drawRoundedRectangle(20, 98, 1160, 470, 4, 1);
    g.setColour(accent.withAlpha(0.25f)); g.drawRoundedRectangle(24, 102, 1152, 462, 3, 0.7f);
    for (const auto corner : { juce::Point<float>(30, 108), { 1170, 108 }, { 30, 558 }, { 1170, 558 } }) diamond(g, corner.x, corner.y, 3);
    label(g, "YURODSTVO", 30, 14, 505, 43, 39, text);
    label(g, "Holy Fool's Pad Engine", 32, 55, 670, 28, 20, accent);
    g.setColour(accent.withAlpha(0.3f)); g.drawLine(705, 69, 712, 69, 0.8f); diamond(g, 720, 69, 4);
    label(g, "VI / TWO OSCILLATORS / FOUR FORMS", 765, 22, 405, 27, 15, muted, juce::Justification::centredRight);
    readout(g, "16 VOICES / SHIMMER / EQ / FX / v0.8.0", 710, 53, 460, 23, 11, muted, juce::Justification::centredRight);
    label(g, "ENGINE / MONITOR", 38, 122, 186, 23, 15, text);
    g.setColour(juce::Colour(0xff282e20)); g.fillRoundedRectangle(38, 157, 186, 206, 3);
    g.setColour(border); g.drawRoundedRectangle(38, 157, 186, 206, 3, 0.7f);
    label(g, "SLOW MATTER", 49, 168, 165, 25, 19, accent);
    for (int osc = 0; osc < 2; ++osc)
        readout(g, "OSC " + juce::String(osc + 1) + ": " + juce::String(yuro::waveformNames[static_cast<size_t>(processor.currentWaveform(osc))]).toUpperCase(),
                49, 204 + osc * 24, 165, 20, 12, text);
    readout(g, "VOICES " + juce::String(processor.voices.load()) + " / " + juce::String(processor.polyphonyLimit()), 49, 255, 165, 20, 12, muted);
    const int type = processor.lastMidiType.load();
    const auto channel = juce::String(processor.lastMidiChannel.load());
    const juce::String midi = type == 0 ? "MIDI: WAITING" : type == 4 ? "PITCH " + juce::String(processor.pitchValue.load()) : type == 3
        ? "CC" + juce::String(processor.lastCC.load()) + ":" + juce::String(processor.lastCCValue.load()) + " CH" + channel
        : juce::String(type == 1 ? "ON " : "OFF ") + "N" + juce::String(processor.lastNote.load()) + " CH" + channel;
    readout(g, midi, 49, 284, 165, 20, 11, accent);
    readout(g, "OUT " + juce::String(juce::Decibels::gainToDecibels(meter, -100.0f), 1) + " dBFS", 49, 315, 165, 20, 12, text);
    readout(g, "DETUNE " + juce::String(processor.currentDetune(), 1) + " ct", 49, 340, 165, 17, 11, accent);
    g.setColour(border); g.fillRect(38, 381, 186, 5);
    g.setColour(accent); g.fillRect(38, 381, juce::roundToInt(186 * juce::jlimit(0.0f, 1.0f, meter)), 5);
    label(g, "POLYPHONY", 38, 407, 186, 23, 15, accent, juce::Justification::centred);
    constexpr std::array<const char*, 6> names { "SHIMMER", "OSC MIX", "ATTACK", "RELEASE", "REVERB", "DELAY" };
    for (int i = 0; i < 8; ++i)
    {
        const int x = knobX[static_cast<size_t>(i % 4)], y = 132 + (i / 4) * 164;
        g.setColour(background.withAlpha(i < 6 ? 0.66f : 0.45f));
        g.fillRoundedRectangle(static_cast<float>(x - 5), static_cast<float>(y - 2), 122, i < 6 ? 138.0f : 128.0f, 5);
        label(g, "E" + juce::String(i + 1) + " / " + (i < 6 ? names[static_cast<size_t>(i)] : "RESERVED"), x, y, 112, 22, 14, i < 6 ? text : muted, juce::Justification::centred);
        if (i >= 6) readout(g, "INACTIVE", x, y + 111, 112, 18, 10, muted, juce::Justification::centred);
    }
    g.setColour(background.withAlpha(0.67f)); g.fillRoundedRectangle(952, 130, 222, 294, 5);
    for (int i = 0; i < 4; ++i)
    {
        const int x = 956 + i * 56;
        constexpr std::array<const char*, 4> faderNames { "MASTER", "BASS", "MID", "TREBLE" };
        label(g, "F" + juce::String(i + 1), x, 133, 54, 20, 15, accent, juce::Justification::centred);
        label(g, faderNames[static_cast<size_t>(i)], x, 154, 54, 16, 12, muted, juce::Justification::centred);
    }
    label(g, "OSCILLATOR I - WAVEFORM", 250, 438, 414, 23, 16, accent);
    label(g, "OSCILLATOR II - WAVEFORM", 700, 438, 414, 23, 16, accent);
    label(g, "PITCH", 26, 583, 67, 22, 14, accent, juce::Justification::centred);
    label(g, "MOD", 100, 583, 67, 22, 14, accent, juce::Justification::centred);
    readout(g, juce::String(processor.currentDetune(), 1) + " ct", 22, 696, 75, 18, 10, text, juce::Justification::centred);
    readout(g, juce::String(processor.modulation.load()), 100, 696, 67, 18, 11, text, juce::Justification::centred);
    readout(g, "RESERVED", 89, 713, 90, 14, 9, muted, juce::Justification::centred);
    label(g, "KEYBOARD / C2 - C4", 210, 583, 340, 22, 15, muted);
    const auto quote = footerQuote();
    const auto split = quote.indexOf(juce::String::fromUTF8(u8" тот будь"));
    label(g, quote.substring(0, split), 32, 734, 1136, 20, 16, accent, juce::Justification::centred);
    label(g, quote.substring(split + 1), 32, 755, 1136, 20, 16, accent, juce::Justification::centred);
}
