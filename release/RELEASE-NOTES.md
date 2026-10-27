# YURODSTVO — Holy Fool's Pad Engine v0.8.0

An experimental synthesizer for ambient music, drones and slowly evolving sound
textures, inspired by Orthodox iconography and the tradition of holy foolishness.
This hobby/noncommercial project is under active development and this release
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

362 automated checks passed on the clean build and again on the VST3 extracted
from its ZIP. The extracted Standalone process started and reported its window;
full interactive/audio smoke testing was not completed. MSVC runtime is statically
linked; normal Windows system libraries are required. No runtime installer is bundled.

## Source and licences

The repository is a reviewed public 0.8.0 snapshot. Original private development
history is retained locally, without rewriting commits or publishing old reference
artwork. The owner's application code is source-available **without an open-source
licence**. Third-party licences remain separate. The official illustration is
owner-authorized AI artwork, not a blanket public-domain declaration.
See LICENSE.txt, THIRD_PARTY.md and the notices included with both downloads.