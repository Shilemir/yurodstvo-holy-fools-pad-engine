# Stable 1.1.0 publication validation

Both Windows x64 Release formats are built from the same clean, reviewed public
commit identified by v1.1.0, extending public v1.0.0 without private ancestry.
The complete Source and Tests trees match the approved local 1.1 implementation.
Publication changes only documentation and distribution materials; no further
sound, preset, interface, parameter-ID, MIDI-map or VST3-identity changes are made.

JUCE 9.0.4/MSVC and the static runtime are retained. A fresh public build and
the extracted VST3 ZIP must pass CTest/technical checks before upload. Packaging
checks version/x64/name, exact embedded PNG/font, static-runtime imports, full
bundle structure, notices, source manifest and ZIP/SHA-256 roundtrips. Measured
results and public-download verification are recorded in the release notes.
TEST_REPORT.md retains the preceding local 685-check run and its limits.

Tests cover exact presets, preserved Hierophant/Custom, fresh Hermit defaults,
actual VST3 saved state, independent keyboard ownership/cancellation and existing
DSP/routing/state/UI behavior. Native Windows keyboard interaction could not be
automated because app-control access was not approved. No physical MiniLab,
Ableton, listening, second-PC, Windows 10/ARM or all-DAW test is claimed.
Unsigned binaries; computer-key rollover depends on hardware.

Only after stable/Latest and anonymous download/hash verification may the owner-
authorized older Releases be removed. Inventory releases first; preserve all
tags, branches, commits and local files. Do not remove unclear releases.