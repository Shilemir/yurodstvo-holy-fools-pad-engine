# Official stable publication — 1.0.0

- Publishes the approved Synth for Arturia MiniLab3 implementation as the stable reference.
- Preserves Hierophant HELL -1.0 dB and the already-applied +2 dB Harmony calibration.
- Updates installation/release metadata and SHA256SUMS; no DSP or feature changes.

# Open-source distribution revision — 2026-10-27

- Original application code relicensed to AGPL-3.0-only with the owner's grant.
- JUCE 9.0.4 uses the AGPLv3 option; third-party notices and OFL font are retained.
- Official AI PNG dedicated under CC0-1.0 to the extent of the owner's rights.
- Portable pinned JUCE fetching, offline source bundle and build documentation.
- Rebuilt 0.8.0 Windows packages include complete legal/source notices.
- Application DSP, UI, plugin identity and MIDI mappings are unchanged.

# Changelog

## 1.0.0 — Stable local baseline

- Changes only THE HIEROPHANT's HELL setting from +4.2 dB to -1.0 dB. The other twelve sound values and the other factory archetypes remain unchanged; THE FOOL (CUSTOM) retains its user-data file.
- Raises both Harmony resonator returns by one common +2.0 dB factor (1.25892541179), preserving the original/recurrent strike ratio, timing, pitch tracking, decay, E7 scale and dry oscillators.
- Sets the technical version to 1.0.0 and derives the editor's v1.0 label from the same CMake version. VST3 identity and existing parameter IDs remain stable. No GitHub release was made.

## 0.15.0 — Artist-calibrated archetypes and inscription repair (local development build)

- Replaces all four factory sounds with the exact artist-selected E1–E8, EQ and waveform values, including 43.7% Hermit Devotion, 93.0% Martyr Madness, 66.6% Inquisitor Madness and -7.1 dB Martyr Heaven. Factory sounds load only when clicked and never change Master, expression or polyphony.
- Replaces the two-column preset grid with four full-width ARCHETYPES rows and a fifth THE FOOL (CUSTOM) / SAVE row. The custom file remains `custom-preset-v1.json` in the same user-data directory.
- Moves and compacts the MIDI/audio monitor, keeps POLYPHONY, PRAY FOR INSPIRATION and SILENCE in the left panel, and tightens the modified-state tolerance.
- Uses the exact ASCII attribution `1 CORINTHIANS 3:18 - KJV` and raises the footer for a complete lower margin. DSP algorithms, VST3 identity and parameter IDs are unchanged. No GitHub release was made.

## 0.14.0 — Biblical inscription and archetypes (local development build)

- Replaces the footer with the exact KJV 1 Corinthians 3:18 text and a small source inscription. The header now shows only the CMake-derived version and removes the unused separator ornament.
- Compacts the MIDI/audio monitor and adds four fixed archetype buttons plus a globally saved CUSTOM slot. SAVE uses a versioned JSON file in the user's application data directory and atomic replacement. Host project state is restored independently.
- After a prayer, preset, state restore or mouse edit, E1–E8 use changes in successive absolute CC values to move the current parameter immediately. Absolute EQ faders retain pickup; a mouse-edited MASTER fader is guarded against jumps.
- Keeps all DSP, VST3 identity, parameter IDs, pad routing, polyphony and effects unchanged. No GitHub release was made.

## 0.13.0 — Recurring bells, tempo sync and control readouts (local development build)

- Raises only subsequent HARMONY bell excitations by about 7.4 dB; keeps the initial strike, continuous resonators and sample-counted 3–9 second timing.
- Synchronizes SOLITUDE's two stereo taps to quarter and dotted-quarter notes from host BPM, with 120 BPM standalone fallback and a 60 ms tap crossfade for tempo changes.
- Simplifies oscillator headings, gives PRAY FOR INSPIRATION the same gold text as SILENCE, and displays selected effect macros and FAITH as one-decimal percentages.
- Displays DUALISM as a bipolar −100% to +100% oscillator balance while preserving its 0–1 parameter, 0.5 default, existing saved values and MIDI mapping.
- Retains VST3 identity, parameter IDs and all other DSP modules; no GitHub release was made.

## 0.12.0 — Polyphony readout and fader order (local development build)

- Starts new on-screen keyboards at MIDI 48 (C2 in the current interface convention), immediately above the reserved pad octave 36–47; saved keyboard positions are retained.
- Shows active engine voices and the selected maximum together in POLYPHONY, including voices in Release, and removes the duplicate monitor line.
- Moves PRAY FOR INSPIRATION directly above SILENCE in the left panel.
- Reorders faders to F1 HELL, F2 EARTH, F3 HEAVEN, F4 MASTER, including the existing physical CC families and mouse attachments.
- Preserves all existing APVTS IDs, parameter values, plugin identity and the parameter-based randomization that excludes MASTER.

## 0.11.0 — Interface cleanup and keyboard navigation (local development build)

- Renames the product to YURODSTVO - Holy Fool's Synth for Arturia MiniLab3 while retaining VST3 class IDs and existing parameter IDs.
- Simplifies visible copy, labels the emergency button SILENCE, and widens the crop of the embedded icon.
- Caps PATIENCE and REMEMBRANCE at 20 seconds, including safe clamping when loading older saved values.
- Adds octave navigation for the mouse keyboard and reserves the pad-control octave from ordinary MIDI and mouse notes.
- Extends MASTER's maximum by about 3 dB and the three EQ bands to ±15 dB; prior plain saved values remain unchanged.
- Normalized host automation and MIDI controller positions map to the extended ranges. No GitHub release was made for this local build.

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
