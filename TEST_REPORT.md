# Local 1.1.0 validation — 2026-10-28

Fresh Windows x64 Release configuration and builds succeeded for Standalone,
VST3 and YurodstvoTests, using JUCE 9.0.4 and MSVC 19.51 with the static MSVC
runtime. `ctest --test-dir build-1.1.0 -C Release --output-on-failure` passed
**685 checks, zero failures** (TechnicalChecks: 36.04 seconds).

- Exact macro, EQ and waveform values pass for the three calibrated archetypes.
  The Hierophant matches the 1.0.0 configuration, including Hell -1.0 dB.
  Factory selection preserves Master, polyphony and expression. Custom save/load
  tests use a project-local fixture and leave the user's persistent file untouched.
- Fresh processor and actual VST3 wrapper defaults match The Hermit. The fresh
  editor selects it without MODIFIED. Arbitrary saved processor state survives
  editor reopening, and actual VST3 wrapper state survives a second hosted instance.
- Computer-key tests cover the MIDI 48/52/55 chord, upper C, held/repeated keys,
  note-off, octave/focus cancellation and release-before-retrigger behavior.
  Shared-engine tests cover overlapping computer/MIDI/mouse ownership and
  exclusion of waveform-pad notes. The VST3 wrapper rejects computer input.
- Existing routing, takeover, state migration, pad, waveform, polyphony, prayer,
  custom sound, EQ, effects, tempo-sync, envelope/glow and signal tests pass.
  Signal tests explicitly restore neutral fixtures so the new musical startup
  defaults do not alter their DSP comparisons.
- The rendered editor image `docs/images/interface-1.1.png` was visually checked:
  v1.1, selected Hermit, calibrated values, compact brown two-row monitor,
  separated headings, no E/F labels, intact illustration and KJV inscription.
  An automated check also confirms the brown monitor and the actual Attack CC tooltip.
- Both final binaries report product/file version 1.1.0, the unchanged full
  product name and x64 architecture. Byte comparisons locate the exact official
  PNG and font inside each binary. Import inspection finds no external MSVC
  runtime DLL requirement. Both VST3 class IDs match 1.0.0. The copied bundle
  preserves its Contents structure and includes the complete dependency notices.

The public 1.1.0 publication extends the existing public 1.0.0 commit
`ac0141dbc0b5d26d4a02f023be520016e0503669`. Historical public tags and commits
are preserved. Fresh public-build and extracted-ZIP measurements are recorded
in release/VALIDATION.md and the published release notes.

Logs are under `artifacts/v1.1-*.log` and
`build-1.1.0/Testing/Temporary/LastTest.log`. Local outputs are under
`artifacts/1.1.0/`. Version and dependency inspection is recorded there.

Limits: native Windows app control was not approved by the computer-use tool,
so real OS key/focus interaction is not claimed as tested. Keyboard behavior
was tested through the state helper and shared processor/engine, not physical
key presses. No physical MiniLab 3, Ableton session, listening evaluation or
second-PC installation test was performed. Simultaneous key count depends on
hardware rollover. Binaries are unsigned. These local checks preceded the fresh public release build.

---

# Official 1.0.0 publication

New publication measurements are recorded in the [v1.0.0 release notes](https://github.com/Shilemir/yurodstvo-holy-fools-pad-engine/releases/tag/v1.0.0).
The historical local validation runs below predate this clean public build.

---

# Stable 1.0.0 local validation — 2026-10-28

Clean Windows x64 Release builds of Standalone, VST3 and the technical test
target succeeded with JUCE 9.0.4 and MSVC. `ctest --test-dir build-1.0-clean
-C Release --output-on-failure` passed **655 checks, zero failures** in 36.11 s.

- A controlled comparison against the actual 0.15.0 VST3 isolated the Harmony
  return by subtracting a render at E7 = 0. The 1.0.0 / 0.15.0 amplitude ratio
  was 1.25893 for both the initial and recurring bell windows, or +2.000 dB.
  The dry oscillator samples matched to within 1e-6. The same deterministic
  MIDI notes and modulation were used for both versions.
- The 16-voice maximum stress render with Harmony, Shimmer, Reverb, Delay,
  maximum EQ boost and Master produced finite samples. Peak reached the
  existing 0.94 safety ceiling in this extreme case. This validates bounded
  output, not a guaranteed uncompressed +2 dB at every final output level.
- Factory archetype checks confirmed THE HIEROPHANT HELL = -1.0 dB, its other
  settings and the remaining factory archetypes. Custom save/load, plugin
  state, prayer, MIDI routing, polyphony, waveform, tempo sync and other
  regression checks passed. Tests used a project-local Custom fixture; the
  user's persistent THE FOOL file was untouched.
- The standalone executable and VST3 module info report 1.0.0. The editor
  screenshot was inspected and shows `v1.0`. The VST3 class ID remains
  `ABCDEF019182FAEB5975726F59643031`. The illustration and font are embedded
  through JUCE binary data, and the packaged VST3 contains the font notice.

The automated tests do not establish behaviour in Ableton Live or with a
physical Arturia MiniLab 3. Those require a final check on the user's setup.

---

# Local artist-calibrated archetype and inscription validation — 0.15.0

Validated on 2026-10-28 with JUCE 9.0.4 and the installed MSVC toolchain.
Windows x64 Release Standalone, VST3 and test targets built successfully.
CTest: **651 checks passed**, zero failures; final run 34.36 seconds.

- All four factory buttons loaded each of the thirteen specified plain parameter
  values with a 0.0002 tolerance, including fractional percentages and dB.
  The active button did not immediately show MODIFIED; a subsequent 0.1%
  manual change did. Inquisitor's 66.6% Madness, +3.4 dB Hell and Saw/Square
  selection were checked through GUI controls.
- THE FOOL retained the 0.14.0 `custom-preset-v1.json` storage path and format.
  Existing Custom save/load and host-state isolation tests passed.
- The final editor screenshot was visually inspected: five full-width rows fit
  above Prayer and Silence; the compact monitor, POLYPHONY and KJV text remain
  readable. The attribution now uses an ASCII hyphen and has lower margin.
- Existing VST3 identity, MIDI, DSP, polyphony, tempo sync, effect, state and
  sixteen-voice regression tests passed. Standalone and VST3 metadata report
  0.15.0 with the unchanged VST3 CID.

Actual Ableton/MiniLab 3 use and subjective listening to the new factory
sounds still require a test on the user's setup.

---

# Local archetype, MIDI and UI validation — 0.14.0

Validated on 2026-10-28 with JUCE 9.0.4 and the installed MSVC toolchain.
Windows x64 Release Standalone, VST3 and test targets built successfully.
CTest: **647 checks passed**, zero failures in the final package build;
test duration 34.27 seconds.

- All four factory sounds set eleven controls and two independent waveforms
  while retaining Master, Doubt, Faith and polyphony. Equal-note renders
  measured a slower Hermit entrance and a brighter Inquisitor spectrum.
- CUSTOM was saved to a versioned JSON test directory, reloaded through a new
  store instance, restored all sound values, and rejected an incompatible file.
  A host state for Hermit was restored independently while global CUSTOM held
  Martyr. The file test used a project-local directory, not user AppData.
- E1–E8 each responded to the first known CC increment after a preset and
  reversed on the next decrement. A previously unknown CC first established a
  safe baseline. A distant physical Master position remained blocked after a
  mouse-edit pickup request.
- Switching among archetypes during sixteen held voices produced finite audio
  within the existing final peak bound without stopping the voices. The
  existing VST3, effect, pad, state and GUI regression checks also passed.
- The rendered editor snapshot was inspected for the exact KJV inscription,
  compact monitor, five slots, button spacing, removed header ornament and
  CMake-derived version number.

Physical MiniLab 3 and Ableton interaction and subjective preset character
still require a listening test on the user's setup. The installed global
CUSTOM directory was not modified by the automated tests.

---

# Local DSP and UI validation — 0.13.0

Validated on 2026-10-28 with JUCE 9.0.4 and the installed MSVC toolchain.
Windows x64 Release Standalone, VST3 and test targets built successfully.
CTest: **606 checks passed**, zero failures; final run 33.51 seconds.

- Stereo Solitude impulse tests found quarter-note and dotted-quarter-note taps
  at 60, 90, 120 and 150 BPM. Invalid BPM preserved the 120 BPM fallback.
  A live 90-to-120 BPM change stayed finite and below the output peak limit.
- Isolated recurring Harmony excitation measured **+7.42 dB** against the
  previous strength. A comparison against the old VST3 showed the first
  strike stayed within -60 dBFS while later output changed measurably.
- New Dualism instances start at the equal 0.5/0.5 mix. The bipolar text
  conversion, legacy 0.62 state round trip and randomization across both
  polarities passed. Five effect readouts use one decimal percent.
- The existing sixteen-voice, effects, MIDI, state and VST3 regression checks
  passed. The editor snapshot was inspected for the renamed oscillator groups
  and matching warm-gold Prayer/Silence text.

The tests cannot establish real-world audibility, perceptual freedom from
tempo-change artifacts or physical MiniLab 3/Ableton behavior. Those need a
user listening test in Ableton. The legacy sample-exact comparisons bypass
Delay because its tempo-dependent timing intentionally changed.

---

# Public release validation — 0.10.1

The new clean-build and extracted-plugin results are recorded in the published
[v0.10.1 release notes](https://github.com/Shilemir/yurodstvo-holy-fools-pad-engine/releases/tag/v0.10.1).
See release/VALIDATION.md for scope. The sections below record earlier development
runs; their counts and timings must not be read as measurements of a new build.

---

# Inspiration / recurring Harmony / Release Glow validation — 0.10.0

Validated on 2026-10-28 with JUCE 9.0.4 and the installed MSVC toolchain.
Windows x64 Release Standalone, VST3 and test targets built successfully.
CTest: **569 checks passed**, zero failures; test duration 33.94 seconds.

- 128 prayers produced distinct configurations with all 13 values inside their
  existing ranges. Both waveform selectors explored all four choices and matching
  pairs. Master, Doubt, Faith, polyphony, legacy detune and routing stayed unchanged.
  Generated values round-tripped through the real processor state.
- GUI prayer activation updates all eight attached knobs, EQ faders and active
  waveform pads, and flashes the single header button. Pickup prevents a distant
  physical CC value from jumping a randomized parameter; crossing it restores
  direct absolute control. Master is excluded from pickup.
- At 44.1/48/96 kHz, Harmony remained measurable with Attack=30 seconds. Sample
  counters produced at least three events over 25 seconds with measured intervals
  inside 3-9 seconds. Note Off stops events; a holding sustain pedal keeps the
  scheduler active until pedal release. Resonance tails then retire without
  hanging voices. A separate signal test verifies that a later shaped strike
  re-excites the actual resonator bank rather than merely incrementing a counter.
- Envelope measurements at Release=0.5/5/30 seconds verify approximately half
  intensity at half Release and zero after completion. Multiple MIDI/mouse owners
  of one pitch and retiring voices contribute to the maximum per-note envelope.
  The processor's published atomic levels also follow those measured values.
- Actual editor and keyboard snapshots were rendered and visually inspected:
  prayer button fits the original header; full/half/finished glow stages use the
  existing warm palette. The window remains 1200 x 780; artwork/font are unchanged.
- Sixteen voices, maximum modules, boosted EQ and moving controllers: 2881.96 ms
  processing per 25600 ms audio (11.26 percent), worst 256-sample block 1.301 ms
  at 48 kHz; output peak 0.94. Thirty prayers over sixteen held notes: 1766.56 ms
  per 16000 ms audio (11.04 percent), peak 0.937228, Master unchanged. These are
  individual offline runs, not DAW CPU or subjective artifact guarantees.
- Existing MIDI/pad/FAITH/pitch/EQ/FX/state regression checks pass. VST3 parameter
  IDs remain compatible. Old Madness reference comparisons intentionally disable
  Harmony because its recurring layer is the deliberate DSP change here.

Local results: `artifacts/0.10.0/TechnicalTests.log`, `Inspiration-stress.txt`,
`Release-glow-states.png` and `YURODSTVO-interface.png`.
No physical MiniLab test, Ableton interaction/listening or another-PC testing was
performed. State tests use processor serialization and an offline VST3 host;
verify recall in your own Ableton set. Existing signal smoothers, crossfades and
peak protection remain in use; listening tests are still needed for subjective
assessment. See INSPIRATION.md for practical controller and listening steps.

---

# Absolute Madness / Faith validation — 0.9.1

Validated on 2026-10-28 with JUCE 9.0.4 and the installed MSVC toolchain.
Windows x64 Release Standalone, VST3 and test targets built successfully.
CTest: **527 checks passed**, zero failures; test duration 28.28 seconds.

- Actual 0.9.0 and new VST3 binaries compared at E8 = 0 / 25 / 50 / 75 / 100
  percent. Old CC1 was held at 127; the new FAITH received changing CC1 events.
  Maximum sample difference was **0 at all five settings**, with sixteen notes,
  Harmony, Shimmer, Reverb, Delay, global pitch gestures and Release. Expression
  smoothing was settled in silence before notes began in the reference build.
- Independent four-note processing tests confirm sample-exact equality while
  simulated MIDI CC1 moves, and while the mouse FAITH strip moves. The indicator
  still updates. No synth/FX parameter is written by CC1. No saved parameter was
  removed: the previous Hesitation was only transient controller state.
- The only audio change is the fixed multiplier 2 on the existing smoothed E8.
  Drift/random interpolation, timbre, amplitude, stereo, Harmony, EQ and FX
  algorithms are unchanged. Existing APVTS IDs and VST3 identity are retained;
  existing state/automation regression tests pass. Older Madness presets now
  deliberately use the old maximum expression depth regardless of old CC1.
- Existing 16-voice maximum stress: 2495.92 ms processing per 25600 ms audio
  (9.75 percent), worst 256-sample block 1.304 ms at 48 kHz; peak 0.94. This is
  one offline test on this machine, not a DAW CPU guarantee. Long-note, Release,
  Panic, EQ, FX, pad, pitch and mouse/MIDI ownership regressions pass.
- Waveform rendering bug: an incorrect `i == 0 || i >= 6` condition restarted
  the path after every point beyond index five. Replaced with complete vector
  geometry for sine, triangle, saw and square. Tests verify one continuous path,
  distinct shapes, full extents and contained bounds at 0.75 / 1 / 2 scales.
  Oscillator sound-generation code was not changed.
- Polyphony +/- callbacks still work. Normal, hover, pressed and disabled
  button colours are distinct and warm; editor/popup selection colours also
  use the panel palette. All four states were rendered and visually inspected.
- The complete 1200 x 780 editor was rendered and visually inspected: clear
  icons on P1-P8, unchanged selection highlighting, FAITH label, warm buttons,
  unchanged official illustration, layout, typography and Cyrillic quote.

Files: `artifacts/0.9.1/TechnicalTests.log`, `Sacred-stress.txt`,
`YURODSTVO-interface.png` and `Polyphony-button-states.png`.
No physical MiniLab testing, Ableton interaction/listening or another-PC testing
was performed. MIDI tests send synthetic messages through the actual processor
and VST3 wrapper. Check physical pads and controller routing in your existing DAW
setup after replacing the complete plugin bundle and rescanning.

---

# Sacred modules validation — 0.9.0

Validated on 2026-10-28 with JUCE 9.0.4 and the installed MSVC toolchain.
Windows x64 Release Standalone, VST3 and technical-test targets built successfully.
CTest: **499 checks passed**, zero failures; suite duration 18.98 seconds.

- Real VST3 0.8.1 vs 0.9.0: every old hosted parameter ID and host display name
  remains available. An actual old plugin state was loaded into the new VST3.
  With the new modules at zero, a chord, pitch gestures, Release and all existing
  effects rendered with maximum sample difference **0** in the measured sequence.
- Default/zero bypass is also sample-exact at 44.1, 48 and 96 kHz, even with CC1
  at maximum and Madness zero. Legacy states reset absent new parameters to zero.
- Modal spectral peaks track two different base notes at all three sample rates,
  including 2.01, 2.756 and 4.07 ratios. The actual sine-voice output acquires
  measurable inharmonic energy when Harmony is enabled.
- Organic trajectories are continuous, bounded and independently evolving over
  fifteen seconds. A held note gains changing stereo output; Hesitation deepens
  its variation without rewriting Madness. Global pitch retains its semitone
  range under maximum Madness/Hesitation.
- User/DAW E7/E8 CCs work in all four pad modes, including conflicting learned
  pad maps. Pending values save before the host-notification timer flush.
  Both new parameters work through mouse controls, automation and actual VST3
  state round trips. Transient Hesitation resets on state load.
- Sixteen voices, Harmony, Madness, all effects, +12 dB EQ, changing pads, Doubt
  and Hesitation: 2419.053 ms processing per 25600 ms audio (9.45 percent), worst
  256-sample block 1.388 ms at 48 kHz. This is one offline run on this machine,
  not a DAW performance guarantee. Peak was bounded at 0.94. All notes completed
  their two-second Release; Panic cleared voice modules and effect tails.
- Macro zero/maximum changes during a held sine note had maximum sample delta
  0.004558; this detects large discontinuities, not every subjective artifact.
- Existing oscillator, pad, CC, mouse ownership, EQ, pitch, FX, polyphony,
  nominal-gain and state regression tests remain passing.
- The editor was rendered and visually inspected: new labels fit the unchanged
  1200 x 780 panel, both new knobs are active, and the icon, quote, pads and
  keyboard remain visible. Existing host names stay technical deliberately;
  the artistic names and explanations are in the editor/tooltips.

Local results: `artifacts/0.9.0/TechnicalTests.log`, `Sacred-stress.txt` and
`YURODSTVO-interface.png`. See SACRED-MODULES.md for design and listening steps.
No physical MiniLab test, Ableton listening/interaction or another-PC test was
performed. The published GitHub release and old build directories are unchanged.

---

# DSP refinement validation — 0.8.1

Validated on 2026-10-28 using the installed MSVC toolchain and JUCE 9.0.4.
Windows x64 Release Standalone, VST3 and test targets built successfully.
CTest passed: **454 checks**, zero failures (12.56 seconds for the suite).
This local DSP iteration has not been uploaded to the published GitHub release.

- Thirty-second impulse measurements at 44.1 / 48 / 96 kHz, for Reverb and
  Delay macros 0 / 0.25 / 0.5 / 0.75 / 1.0. At zero, wet output is exactly zero;
  first-sample dry gains match the documented mix. High settings produce
  stronger wet returns; maximum tails/repeats still have measurable energy at
  10–12 seconds, and the reverb has non-zero energy at 25–30 seconds.
- Sixty seconds with all three maximum effects confirms finite output and
  decay after excitation stops. At low levels, combined effects equal the sum
  of independent returns within 1e-6: no return ducks another.
- Isolated oscillators measured at low/centre/high MIDI Pitch Bend while
  retaining 25 cents of relative detune; measured frequencies agree within
  1.2 Hz with the expected values. Centre and 14-bit endpoint tests pass.
- Regression checks cover pad/CC routing, EQ, host parameter state, temporary
  wheel state, mouse note ownership, sustain, voice stealing, panic and GUI.
  The real built VST3 is loaded by the offline host checks.
- Sixteen voices + maximum effects + boosted EQ: 351.996 ms processing for
  8000 ms audio (4.40 percent), worst 256-sample block 0.635 ms at 48 kHz.
  This is one offline run on this machine, not a DAW performance guarantee.
  Peak remained at or below the existing 0.94 output ceiling. Extreme settings
  can intentionally engage the final soft protection knee.
- Existing nominal-gain regression still passes. The output gain itself was
  not increased in this iteration; legacy test labels still refer to 0.8.

Artifacts: `artifacts/0.8.1/TechnicalTests.log` and
`artifacts/0.8.1/Cathedral-delay-impulses.csv`.
See `DSP-REFINEMENT.md` for design details and an Ableton listening checklist.
No physical MiniLab test, Ableton interaction/audio listening test or another-PC
compatibility test was performed during this iteration.

---

# Release validation — 0.8.0

AGPL distribution validation, 2026-10-27. A fresh Windows x64 Release build was made with official JUCE 9.0.4 and Visual Studio
2026/MSVC 19.51 in a new build directory. Application DSP, MIDI behaviour and
plugin identity were not changed for publication.

## Results

- Standalone EXE, VST3 and technical-test targets built successfully.
- Clean-build CTest: **361 checks passed**, zero failures.
- The VST3 ZIP was extracted into a new directory and that extracted plugin
  passed the same **361 checks**, zero failures.
- Both ZIPs preserve every staged file byte-for-byte; SHA-256 sidecars accompany
  the archives. ZIP paths use forward slashes and retain the complete VST3 tree.
- PE headers identify AMD64/x64. ProductName and ProductVersion are correct.
  VST3 component/controller CIDs match earlier versions.
- Byte-for-byte binary checks find the complete official PNG and Ruslan Display
  font in both EXE and VST3. Assets are embedded, without external file lookups.
- Official builds use `/MT` (static MSVC runtime). `dumpbin /DEPENDENTS` finds
  normal Windows libraries and no external VCRUNTIME/MSVCP runtime DLLs.
- No application source, PDB, OBJ, LIB, EXP, compiler, JUCE checkout, driver or
  runtime installer is included in either binary ZIP. Licence notices are included.
- The earlier pre-conversion Standalone process started and exposed a correctly titled window. No new interactive/audio launch test is claimed for this rebuild.
  The subsequent visual/interactivity smoke test was not completed. This is not
  a claim of successful audio-device setup or listening validation.
- The repository screenshot is rendered from the real JUCE editor, not a mockup.

## Test coverage

Band-limited waveforms, waveform switching, oscillator mix/detune, velocity,
Attack/Release, 1–16 voice management, sustain, stealing, stereo EQ response and
smoothing, octave Shimmer/feedback decay, Reverb/Delay, Master and Panic, level
comparison, MiniLab User/DAW CC routing, pad note/CC selection, parameter/state
roundtrips and actual VST3 hosting are covered.

The on-screen keyboard checks include click/hold/drag/release, black-key hit tests,
focus/visibility/close handling, independent mouse/MIDI ownership of a shared note,
source-aware stealing, transient-state cancellation, bounded-queue overflow and
concurrent producer/consumer release safety. These are automated JUCE callbacks
and offline audio tests, not physical MiniLab or Ableton interaction tests.

The level-comparison check uses the retained private 0.6 binary when available.
This clean public build skipped that one comparison (361 checks versus the earlier 362); the old
binary is not needed to build or use the current instrument.

## Limits

Tests ran on the development Windows 11 x64 computer and a local JUCE VST3 host.
No second computer, Windows 10/ARM, broad DAW compatibility test, physical MiniLab
session or full listening evaluation of this clean distribution is claimed.
The owner reports previous Ableton Live 11 use. Binaries are unsigned.
Normal Windows system libraries and a working audio setup are still required.

## Source/privacy review

The public snapshot includes the current Source, Tests, CMake/build scripts,
English documentation, embedded font with OFL and the owner-authorized AI artwork.
It excludes previous reference images, build outputs, local logs/settings,
attachments, and the original `.git` directory. Original development history is
retained locally without rewriting commits.

A pattern scan of the private text history found no matching GitHub/API/private-key
patterns; the public snapshot is scanned separately before publication. Such scans
are not a mathematical guarantee that every possible secret format is detectable.
The application is now AGPL-3.0-only under the owner's grant. See LICENSE.txt,
THIRD_PARTY.md and assets/ILLUSTRATION-NOTICE.txt.
Portable CMake configuration without JUCE_PATH also succeeded, fetching and
verifying the pinned official JUCE revision. Full Corresponding Source is
provided separately from the binary ZIPs, including JUCE and all project files.
