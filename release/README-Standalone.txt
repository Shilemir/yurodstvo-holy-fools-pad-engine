YURODSTVO - Holy Fool's Pad Engine v@VERSION@
Windows x64 Standalone | Experimental prerelease

START
1. Download the Windows-Standalone ZIP and extract it completely.
2. Run YURODSTVO - Holy Fool's Pad Engine.exe from the extracted folder.
3. If needed, open Options / Audio-MIDI Settings and choose the audio output,
   sample rate/buffer and MIDI input. Settings depend on your installed devices.
4. Start at a moderate Master level and play the C2-C4 mouse keyboard or a MIDI
   controller. No DAW or Arturia controller is required for mouse playback.

CONTROLS
E1 Shimmer / E2 Oscillator Mix / E3 Attack / E4 Release / E5 Reverb / E6 Delay.
E7-E8 Reserved. F1 Master / F2 Bass / F3 Mid / F4 Treble (EQ +/-12dB).
P1-P4 select Sine/Triangle/Sawtooth/Square for oscillatorI; P5-P8 for oscillatorII.
Pitch Bend controls oscillatorII detune +/-100cents; ModCC1 is visual/reserved.
Polyphony1-16, default8. Left-click and hold a key, drag to change notes, release
to start the envelope's Release. Leaving/hiding the keyboard or losing focus
releases its mouse note; physical MIDI ownership is independent. PANIC / SILENCE
clears voices and effects. Existing long Release/reverb tails can persist normally.
This app does not contain a recorder. Save performances with your own audio tools;
the plugin version supports DAW parameter/state storage and automation.

REQUIREMENTS / NOTES
Windows x64 and a working audio output. Built/tested locally on Windows11; other
PCs, Windows10 and Windows on ARM are not independently validated. Unsigned build;
contact the author if blocked rather than disabling protection.
No JUCE, Visual Studio, CMake, compilation or external image/font installation is
required. The MSVC runtime is statically linked; normal Windows DLLs are required.
No driver or runtime installer is bundled. Keep the LICENSES folder with the app.
This experimental instrument is still under development; no universal compatibility
or fitness warranty is offered. See the included licensing/artwork notices.

To uninstall, close the app and remove its extracted folder. No installer runs.