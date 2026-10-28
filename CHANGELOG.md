# Open-source distribution revision — 2026-10-27

- Original application code relicensed to AGPL-3.0-only with the owner's grant.
- JUCE 9.0.4 uses the AGPLv3 option; third-party notices and OFL font are retained.
- Official AI PNG dedicated under CC0-1.0 to the extent of the owner's rights.
- Portable pinned JUCE fetching, offline source bundle and build documentation.
- Rebuilt 0.8.0 Windows packages include complete legal/source notices.
- Application DSP, UI, plugin identity and MIDI mappings are unchanged.

# Changelog

## 0.10.1 — Public Inspiration release (2026-10-28)

- Publishes the implemented 0.8.1–0.10.0 DSP, Inspiration, Harmony, Glow and UI changes.
- Updates release metadata, installation docs and version-aware source packaging.
- Keeps VST3 identity, parameter IDs, MIDI maps and AGPL-3.0-only licensing.
- Includes complete Corresponding Source with JUCE 9.0.4 and SHA-256 sidecars.


## 0.10.0 — Inspiration, recurring Harmony and Release Glow (development milestone; included in 0.10.1)

- Header PRAY FOR INSPIRATION action randomizes 13 existing parameters with host notification and state recall.
- Master, expression, polyphony and routing stay unchanged; randomized hardware controls use pickup.
- Independent per-voice shaped bell excitations recur every 3-9 seconds, including with a 30-second Attack.
- Sustain-aware event retirement and bounded bell release tails retain voice-stealing/Panic safety.
- Per-note atomic envelope snapshots drive warm keyboard glow across Attack, Release, retriggers and retiring tails.
- Existing parameter IDs, VST3 identity, artwork and 1200 x 780 layout retained.

## 0.9.1 — Absolute Madness and Faith (development milestone; included in 0.10.1)

- Madness now always uses the previous full-Hesitation depth, with unchanged drift algorithm and E8 smoothing.
- CC1/Mod Wheel is FAITH: physical/mouse position remains visible, with no DSP connection.
- Fixed broken waveform vector paths; added scale-aware geometry for all four icons.
- Warm brown/olive/gold button states replace blue defaults; text editing and popup highlights use the same palette.
- New actual-VST3 comparisons cover E8 at 0/25/50/75/100 percent against 0.9.0 at CC1=127.

## 0.9.0 — Sacred Harmony and Controlled Madness (development milestone; included in 0.10.1)

- Artistic editor labels and technical tooltips; existing host names/IDs retained.
- HARMONY: six note-tracking resonant modes per voice, harmonic and inharmonic.
- MADNESS: independent smooth pitch, amplitude, timbre and stereo trajectories.
- HESITATION (CC1): bounded transient multiplier of Madness; zero Madness is bypass.
- E7 User/DAW CC19/116, E8 CC16/117; all existing MIDI routes unchanged.
- Stereo voice processing, append-only harmony/madness parameters, schema-8 migration.
- Multi-rate modal/bypass tests, real 0.8.1 VST3 comparison and sixteen-voice stress.

## 0.8.1 — DSP refinement (development milestone; included in 0.10.1)

- Global, smoothed +/-100 cent Pitch Bend for both oscillators, preserving relative detune.
- Eight-line Cathedral Reverb with input diffusion and long, frequency-dependent decay.
- Stronger stereo delay return, up to 88 percent feedback and cross-channel repeats.
- Parallel Shimmer/Reverb/Delay returns without mutual ducking or automatic gain compensation.
- Existing plugin identity, parameter IDs, layout, artwork and MiniLab mappings retained.
- Added multi-rate impulse, global pitch, parallel-routing and sixty-second decay tests.

## 0.8.0

- Official embedded AI illustration with cached, proportionate icon-inspired UI.
- Mouse-playable C2–C4 keyboard: click, hold, drag, release and safe focus/close handling.
- Independent mouse/MIDI note ownership with shared synthesis, voices and effects.
- Existing parameter IDs, VST3 identity, MIDI maps and DSP retained.
- First public source snapshot and separate Windows VST3/Standalone prerelease ZIPs.

## Earlier private development

0.7 introduced octave Shimmer, adjustable1–16voice polyphony, true EQ gain and a
nominal3dB level increase over0.6. Version0.6 introduced Pitch/Mod strips and EQ;
earlier iterations established waveforms, MiniLab routing, stereo Reverb/Delay,
state/automation and the icon-inspired layout. Full private history is retained
locally; it is not included in this public snapshot because of legacy references.
