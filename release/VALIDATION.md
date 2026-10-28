# Release validation — 0.10.1

Both Windows x64 Release formats are built from the same committed public tree
with official JUCE 9.0.4, Visual Studio 2026/MSVC and the static MSVC runtime.
The release tag identifies that exact build commit. The private development
history and historical reference artwork are excluded from public source.

Publication checks:

- Clean Standalone, VST3 and test-target compilation, CTest and extracted-VST3 tests.
- Inspiration's 13 randomized parameters, preserved controls, state and pickup.
- Recurring Harmony, note-off/sustain retirement and real-envelope Release Glow.
- Madness depth, Faith's zero audio effect, waveform paths, MIDI and state tests.
- x64 PE metadata and unchanged VST3 component/controller identity.
- Complete embedded official PNG/font and required third-party/legal notices.
- Complete VST3 bundle, no debug/build files, ZIP roundtrip and SHA-256 sidecars.
- Complete Corresponding Source archive from the same commit with pinned JUCE.
- Uploaded tag/commit, anonymous asset downloads and SHA-256 verification.

The published release notes record measured results from the new build. Earlier
local development results remain in TEST_REPORT.md, explicitly as historical
runs. Optional private comparison binaries are never needed by public builders;
when available they may be used in an additional local regression run and are
never included in the source or binary downloads.

No new Ableton, physical MiniLab, audio listening, other-PC, Windows 10/ARM or
broad DAW test is claimed. Offline automated host/state tests are not a substitute
for listening or checking recall in a user's own DAW. Binaries are unsigned.
