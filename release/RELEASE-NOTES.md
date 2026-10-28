# YURODSTVO — Holy Fool's Pad Engine v0.10.1

Experimental Windows x64 **Pre-release**, succeeding **v0.8.0-agpl**. This release
publishes the implemented 0.8.1–0.10.0 development work with updated release
metadata and packaging. Earlier releases remain available. GNU AGPL v3.0 only.

## Changes since the previous public release

- **PRAY FOR INSPIRATION:** the header button randomizes 13 existing parameters:
  eight macros, three EQ bands and two independent waveform choices. It preserves
  Master, Doubt, Faith, polyphony, legacy detune and MIDI routing. Values notify
  the host and save normally. After a prayer, move each affected absolute MiniLab
  knob/EQ fader through its displayed value to pick it up without a sudden jump.
- **Recurring HARMONY:** six-mode, note-tracking resonances per voice now include
  shaped bell strikes at independent 3–9 second intervals. Bells remain audible
  with long oscillator Attack. Note Off/pedal release stops new strikes; bounded
  resonance tails can continue. Voice stealing and Panic retire them safely.
- **Release Glow:** keyboard illumination follows the actual main voice envelope
  through Attack, Release, sustain, retriggering and voice retirement. It is not
  a separate visual decay. Bell/effect tails can outlast the keyboard glow.
- **MADNESS and FAITH:** E8 uses the full expression depth of the previous local
  0.9.0 at Hesitation = 127. FAITH (Mod Wheel/CC1), moved by MIDI or mouse, remains
  visible with no audio effect. Zero Madness is bypass.
- **Pitch and effects:** DOUBT/Pitch Bend now affects both oscillators globally
  by ±100 cents while preserving their relative detune. Cathedral Reverb has
  diffusion and eight delay lines; Deep Ambient Delay has stronger stereo returns
  and up to 88% feedback. Shimmer, reverb and delay returns operate in parallel.
- **Interface:** repaired, scale-aware waveform icons; warm brown/olive/gold
  control states; artistic labels with technical tooltips. The official embedded
  illustration, Ruslan Display font and 1200 × 780 layout are retained.
- **Controllers and state:** E7 HARMONY uses User/DAW CC19/116; E8 MADNESS uses
  CC16/117. The rest of the MiniLab 3 maps are preserved. Existing VST3 identity,
  parameter IDs and technical host names remain; new macros are appended and
  old states default them to zero. See README.md, INSPIRATION.md and SACRED-MODULES.md.

## Downloads and installation

Under Assets choose:

- **YURODSTVO-Holy-Fools-Pad-Engine-v0.10.1-Windows-VST3.zip**
- **YURODSTVO-Holy-Fools-Pad-Engine-v0.10.1-Windows-Standalone.zip**

Extract everything. For VST3, close the DAW and copy the **entire**
`YURODSTVO - Holy Fool's Pad Engine.vst3` bundle to
`C:\Program Files\Common Files\VST3`, keeping all Contents subfolders; restart
and rescan. Keep only one version in scanned folders. For Standalone, run the
extracted EXE and select output/MIDI devices in Options / Audio-MIDI Settings.
Keep the LICENSES folder. No compiler, JUCE install, separate image/font install
or runtime installer is required. Start at a moderate Master level.

Back up your projects before upgrading: pitch-wheel semantics and effects have
intentionally changed since 0.8.0. Stable IDs preserve parameter addressing;
they do not promise that earlier projects sound identical after these DSP changes.

## Source and licences

The application remains **AGPL-3.0-only**, using JUCE 9.0.4's AGPLv3 option.
Commercial use is permitted under the applicable licences. Preserve notices and
provide complete Corresponding Source when conveying binaries; modified covered
works also have the applicable AGPL sections 5 and 13 obligations.

The **YURODSTVO-Holy-Fools-Pad-Engine-v0.10.1-Corresponding-Source.zip** asset
contains the exact project revision, scripts, assets and official JUCE 9.0.4
source (commit `3fb5cbc0340758596dac39f7c8de47d562b4896e`) for offline builds.
All three archives have SHA-256 sidecars. Automatic GitHub source archives contain
the project only; CMake can fetch pinned JUCE or use JUCE_PATH. See BUILDING.md.

Third-party licences/notices remain intact, including MIT VST3 SDK and OFL font.
Only the owner's applicable rights in the official AI PNG are dedicated under
CC0-1.0, with its documented AI/provenance limitations. See LICENSE.txt,
COPYRIGHT.md, THIRD_PARTY.md and assets/ILLUSTRATION-NOTICE.txt.

## Validation and known limits

The publication process builds both formats from one committed source revision,
runs CTest and the extracted VST3 checks, validates x64/version/imports and exact
embedded PNG/font bytes, and verifies ZIP contents and public download hashes.
Measured results are appended to the published release notes after validation.

Unsigned experimental binaries, tested on the development Windows x64 machine
with an offline JUCE VST3 host. No new Ableton, physical MiniLab, listening,
other-PC, Windows 10/ARM or all-DAW validation is claimed. Listen and check project
recall in your own host. High macro/EQ settings can engage output protection.
