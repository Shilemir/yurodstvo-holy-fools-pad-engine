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
E1 DEVOTION (Shimmer), E2 DUALISM (Mix), E3 PATIENCE (Attack), E4 REMEMBRANCE (Release), E5 SANCTUARY (Reverb), E6 SOLITUDE (Delay).
E7 HARMONY (Modal Resonance), E8 MADNESS (Organic Instability). F1 MASTER / F2 HELL / F3 EARTH / F4 HEAVEN (EQ +/-12dB).
P1-P4: Sine, Triangle, Sawtooth, Square for oscillatorI.
P5-P8: the same four waveforms for oscillatorII.
DOUBT (Pitch Bend): global +/-100 cents for both oscillators; relative detune is preserved. FAITH (CC1) is visual only.
MADNESS uses the previous full-Hesitation depth; FAITH (Mod CC1) has no audio effect.
Polyphony:1-16, default8. Mouse keyboard: left-click, hold, drag, release.
Mouse and physical MIDI notes are tracked independently and share polyphony.
Leaving the keyboard or losing focus releases its mouse note. Long Release can
continue sounding after release. PANIC / SILENCE clears voices and effect tails.
Mouse notes are not sent to the DAW as MIDI output; record physical MIDI for clips.
User and DAW absolute MiniLab3 CC maps are supported; the DAW must forward input.

LICENSING AND SOURCE
Free and open-source software: GNU AGPL v3.0 only (AGPL-3.0-only).
You may use, study, modify and redistribute it, including commercially, subject
to the licence. Include Corresponding Source and notices when conveying binaries;
mark your modifications. Modified remote network versions have section 13 source
offer obligations. See LICENSES/LICENSE.txt and LICENSES/THIRD_PARTY.md.
Complete source with JUCE 9.0.4 is available at no charge from the same release:
https://github.com/Shilemir/yurodstvo-holy-fools-pad-engine/releases/tag/v0.10.1
See SOURCE-CODE.txt and the Corresponding-Source.zip asset for offline building.
The unmodified font stays OFL-1.1. Only the official AI PNG is dedicated under
CC0-1.0, to the extent of the owner's rights; see LICENSES/ILLUSTRATION-NOTICE.txt.

PRAY FOR INSPIRATION randomizes E1-E8, EQ and both waveforms. Master, expression and polyphony are unchanged. After a prayer move each knob/EQ fader through its displayed value to pick it up. Harmony has recurring bell strikes independent of Attack; keyboard glow follows the main voice envelope.
