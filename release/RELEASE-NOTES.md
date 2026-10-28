## About YURODSTVO

YURODSTVO — Holy Fool's Synth for Arturia MiniLab3 is a free, open-source ambient
synthesizer inspired by Orthodox iconography, the tradition of holy foolishness
and the sound of church bells. It explores sacred acoustic space, drones and
unstable, slowly evolving textures.

## Features

- Dual oscillators with four selectable waveforms each.
- Up to 16 voices of polyphony.
- Devotion — Shimmer; Dualism — Oscillator Balance.
- Patience — Attack; Remembrance — Release.
- Sanctuary — Cathedral Reverb; Solitude — BPM-synchronized Delay.
- Harmony — Recurring Bell Resonances; Madness — Controlled Instability.
- Hell, Earth, Heaven — Three-band EQ, plus global Master Volume.
- Pray for Inspiration — Sound Randomization.
- Five Archetypes: The Hermit (Contemplation), The Martyr (Suffering),
  The Inquisitor (Judgment), The Hierophant (Transcendence), and
  The Fool (Custom), which saves and recalls your own sound configurations.
- Interactive keyboard with Release Glow and octave navigation.
- Arturia MiniLab 3 MIDI integration with existing User and DAW CC routes.

## What's New in v1.1

- Recalibrated factory archetypes: The Hermit, The Martyr and The Inquisitor,
  using the artist's exact settings. The Hierophant remains unchanged,
  including Hell at -1.0 dB; existing Custom storage is preserved.
- The Hermit is now the default sound for new instances. Saved host state
  takes precedence, and reopening the editor does not reset the sound.
- Computer keyboard support in Standalone: A/W/S/E/D/F/T/G/Y/H/U/J/K plays
  chromatically from the visible C (initially MIDI 48, C2). Existing octave
  arrows move the range. Held keys do not repeat; focus/range changes cancel
  notes. VST3 does not capture these computer keys.
- Cleaner interface without visible E1–E8/F1–F4 labels, improved spacing
  between Polyphony and Archetypes, and a compact warm brown MIDI/audio monitor
  showing only MIDI event/note and output dBFS.
- Minimalist English tooltips with verified controller information.
- Existing DSP algorithms, Harmony's previously applied +2 dB bell calibration,
  MIDI mappings, APVTS parameter IDs, VST3 identity and licensing are preserved.

## Installation

Windows x64. Verify the ZIPs against `SHA256SUMS.txt`, then extract everything.

**VST3:** close your DAW and copy the entire
`YURODSTVO - Holy Fool's Synth for Arturia MiniLab3.vst3` bundle to
`C:\Program Files\Common Files\VST3`. Preserve all Contents subfolders.
Restart/rescan your 64-bit VST3 host. Avoid duplicate scanned installations.

**Standalone:** run the extracted EXE and select your audio/MIDI devices in
Options / Audio-MIDI Settings. Keep the LICENSES folder beside the application.
Play with MIDI, the mouse keyboard, or the computer keyboard while the editor
is active. End users do not need JUCE, Visual Studio or external image/font files.
Start with a moderate Master/output level. Binaries are unsigned.

## Licensing

Original application code is **GNU AGPL v3.0 only (AGPL-3.0-only)**. JUCE 9.0.4
uses its AGPL option. Preserve notices and provide the matching complete
Corresponding Source when redistributing binaries. The source for these binaries
is tagged **v1.1.0** in this repository; the accompanying Corresponding-Source
ZIP also includes the pinned unmodified JUCE source for offline builds.

VST3 SDK MIT notices, the Ruslan Display OFL font licence and all other dependency
notices are retained. Only the owner's applicable rights in the official AI PNG
are dedicated under CC0-1.0, subject to the documented provenance/rights limits.
The KJV inscription keeps its attribution and regional-rights notice.
See LICENSE.txt, THIRD_PARTY.md and the notices inside both binary packages.

## Validation and limits

The preceding local 1.1.0 build passed 685 automated checks. Fresh public-build,
extracted-ZIP and download measurements are appended when publication is verified.
No new physical MiniLab, Ableton, listening, second-PC or all-DAW test is claimed.
Native OS keyboard/focus interaction was not automated because app-control
access was not approved; keyboard logic/shared-engine behavior is tested offline.
Simultaneous computer keys depend on keyboard hardware rollover.