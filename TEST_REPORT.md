# Release validation — 0.8.0

A fresh Windows x64 Release build was made with JUCE 9.0.4 and Visual Studio
2026/MSVC 19.51 in a new build directory. Application DSP, MIDI behaviour and
plugin identity were not changed for publication.

## Results

- Standalone EXE, VST3 and technical-test targets built successfully.
- Clean-build CTest: **362 checks passed**, zero failures.
- The VST3 ZIP was extracted into a new directory and that extracted plugin
  passed the same **362 checks**, zero failures.
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
- The extracted Standalone process started and exposed a correctly titled window.
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
A fresh public clone without that old binary skips that one comparison; the old
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
The author chose source availability without an open-source licence. See LICENSE.txt,
THIRD_PARTY.md and assets/ILLUSTRATION-NOTICE.txt.