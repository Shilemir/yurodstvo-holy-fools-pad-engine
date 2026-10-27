#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <atomic>
#include "ControlRouting.h"

class PadRouter
{
public:
    PadRouter() { for (auto& value : learned) value.store(-1); }
    // 0=Notes, 1=CC, 2=MiniLab Auto, 3=Learned. Existing mode indices are stable.
    // Return -1 for ordinary MIDI, -2 for consumed releases/learning, 0..7 for hits.
    int handle(const juce::MidiMessage& message, int mode, int channel, int firstNote, int firstCC)
    {
        const int command = learningCommand.exchange(0);
        if (command == 1) learningStep.store(0);
        if (command == 2) { learningStep.store(-1); completed.store(false); }
        const bool note = message.isNoteOnOrOff(), cc = message.isController();
        if (!note && !cc) return -1;
        const int number = note ? message.getNoteNumber() : message.getControllerNumber();
        // A broad manual range or restored learned map must never swallow
        // synthesis, effects, master, sustain or panic controllers.
        if (cc && yuro::protectedController(number)) return -1;
        const bool press = note ? message.isNoteOn() : message.getControllerValue() > 0;
        const int encoded = encode(note ? 1 : 2, message.getChannel(), number);
        const int step = learningStep.load();
        if (step >= 0 && step < 8)
        {
            if (press)
            {
                bool duplicate = false;
                for (int i = 0; i < step; ++i) duplicate = duplicate || scratch[static_cast<size_t>(i)] == encoded;
                if (!duplicate)
                {
                    scratch[static_cast<size_t>(step)] = encoded;
                    if (step == 7)
                    {
                        for (size_t i = 0; i < learned.size(); ++i) learned[i].store(scratch[i]);
                        completed.store(true);
                    }
                    learningStep.store(step + 1);
                }
            }
            return -2;
        }
        int pad = -1;
        if (mode == 3)
        {
            for (size_t i = 0; i < learned.size(); ++i) if (learned[i].load() == encoded) { pad = static_cast<int>(i); break; }
        }
        else if (channel == 0 || channel == message.getChannel())
        {
            if (note && (mode == 0 || mode == 2)) pad = number - firstNote;
            if (cc && (mode == 1 || mode == 2)) pad = number - firstCC;
            if (pad < 0 || pad >= 8) pad = -1;
        }
        if (pad < 0) return -1;
        if (!press) return -2;
        lastPad.store(pad); lastType.store(note ? 1 : 2); lastNumber.store(number);
        lastChannel.store(message.getChannel()); hitCount.fetch_add(1);
        return pad;
    }
    void startLearning() { learningCommand.store(1); }
    void cancelLearning() { learningCommand.store(2); }
    int learningStatus() const
    {
        const int command = learningCommand.load();
        return command == 1 ? 0 : command == 2 ? -1 : learningStep.load();
    }
    static int encode(int type, int channel, int number) { return (type << 12) | (channel << 7) | number; }
    std::array<std::atomic<int>, 8> learned;
    std::atomic<int> learningStep { -1 }, lastPad { -1 }, lastType { 0 }, lastNumber { -1 }, lastChannel { 0 }, hitCount { 0 };
    std::atomic<bool> completed { false };
private:
    std::array<int, 8> scratch {};
    std::atomic<int> learningCommand { 0 };
};
