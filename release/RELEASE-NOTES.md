# YURODSTVO — Holy Fool's Pad Engine v0.8.0

An experimental synthesizer for ambient music, drones and slowly evolving sound
textures, inspired by Orthodox iconography and the tradition of holy foolishness.
This open-source project is under active development and this release
is marked **Pre-release**.

## Features

- Two oscillators, each with Sine, Triangle, Sawtooth and Square waveforms.
- Selectable 1–16 voice polyphony, default 8; velocity, sustain and slow envelopes.
- Stereo three-band EQ, octave Shimmer, Reverb and Delay.
- Independent waveform pads, MiniLab 3 knob/fader mapping, oscillator II Pitch
  Detune, parameter automation and saved plugin state.
- Mouse-playable C2–C4 keyboard and embedded official AI illustration/font.
- Mod Wheel is currently visual/reserved; E7 and E8 are inactive.

## Downloads

Choose the binary ZIP you need under **Assets**, not the automatic source archive:

- **YURODSTVO-Holy-Fools-Pad-Engine-v0.8.0-Windows-VST3.zip**
- **YURODSTVO-Holy-Fools-Pad-Engine-v0.8.0-Windows-Standalone.zip**

SHA-256 files are supplied for checking download integrity.

## Installation

**VST3:** extract the complete archive, close your DAW and copy the entire
`YURODSTVO - Holy Fool's Pad Engine.vst3` folder into
`C:\Program Files\Common Files\VST3`. Preserve all Contents subfolders. Restart
and rescan VST3 plugins, then load the instrument on a MIDI/instrument track.
Do not scan multiple versions of this same plugin at once.

**Standalone:** extract the archive, run the EXE and, if needed, select audio/MIDI
in Options / Audio-MIDI Settings. Keep the LICENSES folder with the app.

No JUCE, Visual Studio, compilation or separate image/font installation is needed.
Start with a moderate Master level. See the included English README for controls.

## MIDI

Both absolute MiniLab 3 User and DAW CC maps are supported. P1–P4 select oscillator I
waveforms; P5–P8 select oscillator II waveforms. Pitch Bend controls oscillator II
detune, not global pitch. The DAW must forward notes, CC and Pitch Bend. Mouse
notes use the same synthesis engine but are not emitted as MIDI output for clips.

## Requirements and validation

Windows x64. VST3 requires a 64-bit VST3 instrument host; Standalone requires a
working audio output. Built/tested on Windows 11; the owner reports previous
Ableton Live 11 operation. Other computers, Windows 10/ARM and all other DAWs
have not been independently validated. Binaries are unsigned.

361 automated checks passed on the AGPL clean build and again on the VST3 extracted
from its ZIP. The historical 0.6 comparison is skipped in public builds (the earlier private build passed 362). The previous pre-conversion Standalone process started and reported its window;
full interactive/audio smoke testing was not completed. MSVC runtime is statically
linked; normal Windows system libraries are required. No runtime installer is bundled.

## Open-source conversion: v0.8.0-agpl

The original application is now **GNU AGPL version 3 only (AGPL-3.0-only)**,
including the original 0.8.0 code. You may use, study, modify and redistribute it,
including commercially, under that licence. JUCE 9.0.4 uses its AGPLv3 option;
previous Starter/source-available notices are superseded for this distribution.
Third-party copyrights, SDK licences and the font's OFL-1.1 remain intact.

This is a licensing, build-portability and documentation revision of application
**0.8.0**. The original v0.8.0 tag is preserved; **v0.8.0-agpl** identifies the
open-source source revision. DSP, UI, plugin identity and MIDI mappings are
unchanged. Windows binaries have been rebuilt from the open-source revision.
ZIPs and SHA-256 files have been regenerated with complete legal/source notices.

Download **YURODSTVO-Holy-Fools-Pad-Engine-v0.8.0-AGPL-Corresponding-Source.zip**
and its checksum from Assets for the complete Corresponding Source, including
unmodified JUCE 9.0.4 and its vendored dependencies. It builds offline with the
tools in BUILDING.md. GitHub's automatic source archives contain the project
only; their CMake configuration fetches pinned JUCE or accepts JUCE_PATH.

When conveying binaries, provide complete Corresponding Source under AGPLv3
section 6, preserve notices and identify modifications. Modified covered works
must remain AGPLv3; modified versions used for remote network interaction must
provide the section 13 source offer. There is no noncommercial restriction.
Contributions are welcome. See LICENSE.txt, COPYRIGHT.md and THIRD_PARTY.md.

Only the owner's applicable rights in assets/yurodstvo_icon.png are dedicated
under CC0-1.0. This AI illustration carries no claim of exclusive copyright or
guaranteed third-party-rights clearance. Font, libraries and the full interface
screenshot are excluded from CC0; see assets/ILLUSTRATION-NOTICE.txt.
