# Licence audit: 0.8.0 AGPL distribution

Review date: 2026-10-27. Scope: the reviewed public snapshot, Windows x64
VST3/Standalone build configuration, dependency sources and release ZIPs.
This records the licensing review and selected grants, not a guarantee of legal
outcomes in every jurisdiction or an audit of future forks.

## Evidence and result

The previous LICENSE.txt reserved application rights and THIRD_PARTY.md elected
JUCE Starter. They are superseded by the owner's explicit AGPL-3.0-only grant
in COPYRIGHT.md. No DSP, UI, parameter or MIDI changes are required.

Official JUCE tag 9.0.4 resolves to
`3fb5cbc0340758596dac39f7c8de47d562b4896e`. A fresh upstream checkout was
obtained. 3885 files in modules/, extras/Build/, CMakeLists.txt, LICENSE.md and
JUCE.spdx.json were compared with the JUCE tree used for the original binary
build: no differences beyond line endings. The module header declares 9.0.4
and an AGPLv3 alternative; the upstream SPDX inventory identifies that option
as AGPL-3.0-only. See the [exact licence](https://github.com/juce-framework/JUCE/blob/9.0.4/LICENSE.md)
and [inventory](https://github.com/juce-framework/JUCE/blob/9.0.4/JUCE.spdx.json).
The commercial EULA is an alternative, not the selected basis of this release.

The project links JUCE audio_utils and its transitive audio, graphics, GUI,
events, data and core modules plus the PNG/font. C++ includes and CMake targets
revealed no additional vendored application library. VST3 SDK 3.8.0's root and
subdirectory licences are MIT. Notices listed in THIRD_PARTY.md were checked
and retained. Their permissive copyright/patent conditions do not prohibit this
AGPLv3 distribution. Apache-2.0 is compatible with the version-3 copyleft path;
the IJG acknowledgement, libwebp patent text and Opus notices are preserved.

Ruslan Display is an unmodified separate font under OFL-1.1, which permits
embedding/bundling while retaining its licence and reserved-name conditions.
It is not relicensed under AGPL. See the [OFL FAQ](https://openfontlicense.org/ofl-faq/).
PreSonus headers retain their own public-domain statements and disclaimers.

Only Standalone and VST3 are built. ASIO, Windows MIDI Services, AAX, AU, LV2,
external LAME/FFmpeg, webview and Android integrations are not enabled. The full
JUCE source archive includes unused components under their own notices as
aggregation, not as a claim they are linked or owned by this project. Optional
ASIO/AAX source offers GPLv3 alternatives; no proprietary SDK grant is inferred.
MSVC, Windows SDK and Windows system libraries are tools/System Libraries;
their licences are not replaced. No Microsoft DLL bundle or driver is added.

## Source delivery and limits

AGPL sections 5 and 6 require preserving notices, identifying modifications and
providing Corresponding Source when conveying object code. Section 13 adds the
modified remote-network-use source offer. Downloads point to a complete source
archive on the same GitHub release: project files, scripts, assets and exact
JUCE source. Compilers and operating-system source are not included. Portable
CMake dependency fetching and offline discovery replace the local-path default.

The original v0.8.0 tag is retained without rewriting it. Tag v0.8.0-agpl
identifies the licensing/build-documentation revision; application version stays
0.8.0. The grant also covers the identical original 0.8.0 application code.
Release validation records the build/package checks and downloaded hashes.
Private Git history and external reference artwork remain excluded.

The owner elects CC0-1.0 for the AI PNG only, to the extent of rights they can
waive or license. [CC0](https://creativecommons.org/publicdomain/zero/1.0/legalcode.en)
includes a fallback licence and limitations; it does not establish exclusive
rights or clear third-party rights. The owner's provenance statement is recorded.
The generation-service contract and full generation record were not supplied
for independent verification; no such clearance or universal copyright claim
is made. This limitation adds no restriction to the owner's grant.

No licence incompatibility was identified preventing distribution of the
reviewed instrument and dependencies under AGPLv3 with the separate font/artwork
grants and preserved notices. This does not resolve every possible proprietary
DAW/plugin derivative-work boundary or grant a JUCE linking exception. Assess
additional code or a bundled proprietary host separately.
