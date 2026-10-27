YURODSTVO - Holy Fool's Pad Engine v@VERSION@
Windows x64 VST3 | Experimental prerelease

INSTALLATION
1. Download the Windows-VST3 ZIP and extract it completely.
2. Close your DAW. Copy the ENTIRE folder
   YURODSTVO - Holy Fool's Pad Engine.vst3
   into C:\Program Files\Common Files\VST3
   Windows may request administrator permission for this copy.
3. Preserve Contents/x86_64-win and Contents/Resources. Do not flatten the bundle
   or copy only the inner .vst3 binary.
4. Open your DAW, enable VST3 system folders and rescan plugins.
5. Insert YURODSTVO - Holy Fool's Pad Engine on a MIDI/instrument track.
   In Ableton Live 11: Preferences > Plug-Ins > enable VST3 system folders,
   click Rescan, then load the instrument. Check v@VERSION@ in the editor.
6. Start at a moderate Master level. Play MIDI or click the C2-C4 keyboard.

UPGRADING / UNINSTALLING
Do not keep multiple versions of this same instrument in scanned folders: its
VST3 identity is stable. Back up your DAW project before replacing an older
bundle. To uninstall, close the DAW, remove this bundle, then rescan.

REQUIREMENTS
Windows x64 and a 64-bit VST3 instrument host. Built/tested locally on Windows11;
earlier versions were reported working by the owner in Ableton Live11. This clean
release was checked in an automated VST3 host, not on a second PC or every DAW.
Windows10 and Windows on ARM are not independently validated. The binary is
unsigned. If Windows blocks it, contact the author; do not disable protection.
No JUCE, Visual Studio, CMake, compilation, external PNG or font installation is
required. The MSVC runtime is statically linked; inspected imports require normal
Windows libraries, not a separate VCRUNTIME/MSVCP DLL installation.

BASIC CONTROLS
E1 Shimmer / E2 Oscillator Mix / E3 Attack / E4 Release / E5 Reverb / E6 Delay.
E7-E8 Reserved. F1 Master / F2 Bass / F3 Mid / F4 Treble (EQ +/-12dB).
P1-P4: Sine, Triangle, Sawtooth, Square for oscillatorI.
P5-P8: the same four waveforms for oscillatorII.
Pitch Bend: oscillatorII detune +/-100cents, centre0; not global pitch bend.
ModCC1: visual0-127, reserved, no audio destination.
Polyphony:1-16, default8. Mouse keyboard: left-click, hold, drag, release.
Mouse and physical MIDI notes are tracked independently and share polyphony.
Leaving the keyboard or losing focus releases its mouse note. Long Release can
continue sounding after release. PANIC / SILENCE clears voices and effect tails.
Mouse notes are not sent to the DAW as MIDI output; record physical MIDI for clips.
User and DAW absolute MiniLab3 CC maps are supported; the DAW must forward input.

Keep the accompanying licences and notices. This is source-available software,
not an open-source grant for the original application code. The official artwork
is AI-generated and authorized by its provider; no public-domain claim is made.