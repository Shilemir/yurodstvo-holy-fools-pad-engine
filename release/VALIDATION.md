# Stable 1.0.0 publication validation

Both Windows x64 Release formats are built from one committed reviewed public
tree, identified by v1.0.0. JUCE 9.0.4/MSVC and static runtime are retained.
The C++ Source and Tests trees are identical to the approved local 1.0.0 tag;
only publication documentation, source export and packaging are prepared here.

The release process checks clean compilation, CTest and the extracted VST3;
five archetypes/Custom persistence; approved Hierophant and Harmony calibration;
quote/attribution, version label, keyboard, tempo sync, MIDI/state and existing
VST3 identities. x64 metadata, exact embedded PNG/font, complete bundle paths,
licences, source manifest, SHA-256 and public downloads are verified.
Measured counts/results are appended to the published release notes. TEST_REPORT.md
also preserves historical local runs, which are not new publication measurements.

Optional local reference binaries can be used in a separate regression run but
are not required or distributed. Custom tests use project-local fixtures and do
not change the user's THE FOOL file. No new Ableton, physical MiniLab, listening,
other-PC, Windows 10/ARM or all-DAW testing is claimed. Binaries are unsigned.

Old Releases may be removed only after stable/latest and download verification.
Their Git tags/commits and local project/history must remain unchanged.
