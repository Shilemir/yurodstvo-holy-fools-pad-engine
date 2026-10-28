# YURODSTVO v1.0.0 — Holy Fool's Synth for Arturia MiniLab3

The first official stable release of YURODSTVO: an independent instrument inspired
by the tradition of holy foolishness, Orthodox iconography, church bells and
sacred space. Designed for experimental ambient, drones and unstable evolving
textures, with direct Arturia MiniLab 3 integration.

## The instrument

- Dual oscillator synthesizer; four independently selectable waveforms per oscillator.
- **Devotion** — Shimmer; **Dualism** — Oscillator Balance.
- **Patience** — Attack; **Remembrance** — Release.
- **Sanctuary** — Cathedral Reverb; **Solitude** — BPM-synchronized Delay.
- **Harmony** — Recurring Bell Resonances; **Madness** — Controlled Instability.
- Three-band EQ: **Hell, Earth, Heaven**, plus global **Master Volume**.
- Polyphony up to 16 voices, interactive keyboard with octave navigation,
  Release Glow, host automation and saved plugin state.
- **Pray for Inspiration** — unpredictable parameter combinations, preserving
  Master/expression/polyphony, rather than a fixed preset selection.
- Five Archetypes: **THE HERMIT — Contemplation**, **THE MARTYR — Suffering**,
  **THE INQUISITOR — Judgment**, **THE HIEROPHANT — Transcendence** and
  **THE FOOL (CUSTOM) — User-defined sound**. SAVE stores a personal configuration
  for the Custom slot; host projects keep their own state independently.

This publishes the existing approved 1.0 sound and UI. THE HIEROPHANT has HELL
at **−1.0 dB**. Harmony's initial and recurring bell returns already include the
approved **+2 dB** calibration. No additional DSP change is made for publication.
Technical version: **1.0.0**; editor label: **v1.0**. Existing VST3 identity and
parameter IDs remain intact. Hardware faders now follow F1 Hell / F2 Earth /
F3 Heaven / F4 Master; see README before upgrading projects from early 0.x builds.

## Download

- **YURODSTVO-v1.0.0-Windows-x64-VST3.zip**
- **YURODSTVO-v1.0.0-Windows-x64-Standalone.zip**
- **SHA256SUMS.txt** — checksums for the binary and complete source archives.

Extract everything. For VST3, close the DAW and copy the entire
`YURODSTVO - Holy Fool's Synth for Arturia MiniLab3.vst3` bundle to
`C:\Program Files\Common Files\VST3`, retaining all Contents folders; rescan.
Keep only one version in scanned folders. For Standalone run the extracted EXE
and select audio/MIDI devices in Options / Audio-MIDI Settings. Keep LICENSES.
No JUCE, compiler or separate font/image installation is needed. Back up old
projects and begin at a moderate monitoring level.

## Open source

**GNU AGPL v3.0 only (AGPL-3.0-only)**, using JUCE 9.0.4's AGPL option.
The **YURODSTVO-v1.0.0-Corresponding-Source.zip** asset provides the exact project
revision and unmodified official JUCE source for offline builds. Preserve notices
and provide Corresponding Source when redistributing binaries. Third-party terms,
OFL font and the owner's limited CC0 dedication for the AI illustration remain
unchanged. See LICENSE.txt, THIRD_PARTY.md and BUILDING.md.

## Validation and limits

Both Windows x64 Release formats are built from one tagged source commit.
Automated build, extracted-plugin, archetype, state/MIDI, embedded-asset and ZIP
checks are performed before publication; measured results are appended below.
Binaries are unsigned. No new Ableton, physical MiniLab, listening, other-PC,
Windows 10/ARM or all-DAW validation is claimed. Without valid host BPM, Solitude
uses the last valid tempo, initially 120 BPM. Independent bell/effect tails can
outlast the main voice's keyboard glow. Extreme settings can reach output protection.
