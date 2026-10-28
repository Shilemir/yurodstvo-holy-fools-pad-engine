# Building YURODSTVO 1.0.0

## Windows x64 toolchain

- Visual Studio 2026 with Desktop development with C++ and a Windows SDK
  (tested: MSVC 19.51), or Visual Studio 2022 with its C++ tools.
- CMake 3.22 or newer, and a version supporting your generator. Visual Studio
  2026 requires CMake 4.2 or newer. Add CMake/CTest and Git to PATH.
- Git for the initial JUCE download; C++17 support.

The application, Windows resources, PNG, font, tests and CMake files are all in
the repository. No Projucer project generation, JUCE account or JUCE commercial
licence is needed for this AGPL distribution. JUCE is **9.0.4**, upstream commit
`3fb5cbc0340758596dac39f7c8de47d562b4896e`.

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>'
cmake --build build --config Release --target Yurodstvo_Standalone Yurodstvo_VST3 YurodstvoTests --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

For Visual Studio 2022 use `-G "Visual Studio 17 2022"` in a new build directory.
The VS 2022 path is provided for portability but was not tested for this release.
For other platforms, JUCE supports additional generators and dependencies;
this release's scripts and validation target Windows x64 only.

CMake downloads the official JUCE release via a shallow Git checkout and checks
its exact commit before adding it. No sibling directory or machine-specific
drive is assumed. To use an already obtained, unmodified JUCE 9.0.4 tree:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DJUCE_PATH="C:/dependencies/JUCE-9.0.4"
```

Use a new build directory when switching dependency locations. Other JUCE
versions are rejected. No third-party SDK purchase is required for these VST3
and Standalone targets. ASIO is disabled by JUCE's default configuration;
external ASIO drivers/SDKs and AAX are outside this build.

## Offline Corresponding Source

Download `YURODSTVO-v1.0.0-Corresponding-Source.zip`
and its SHA-256 sidecar from the same release as the binaries. Extract it and
run the same commands above from the project root. The archive includes the
exact official JUCE source at `dependencies/JUCE`, automatically discovered by
CMake. It does not require Git or network access to configure. Compilers,
CMake and Windows SDK must already be installed. The archive also contains
all upstream licence notices and the unmodified font and illustration.

Executables are under `build/Yurodstvo_artefacts/Release/Standalone/` and
`build/Yurodstvo_artefacts/Release/VST3/`. Preserve the entire .vst3 bundle.
Tests write reports under artifacts/. A private historical 0.6 binary is
optional; its absence skips only the historical comparison.

## Distribution

`./release/build-distribution.ps1` locates installed Visual Studio C++ tools,
builds in a fresh directory, runs CTest and packages both formats. Optional
parameters: `-BuildDirectory` and `-JuceDirectory`; with neither dependency
override nor bundled JUCE, CMake fetches the pinned release.

`./release/package-windows.ps1 -BuildDirectory <build>` inspects x64/version,
static runtime imports and embedded assets, includes legal notices and a source
download pointer, creates ZIPs and SHA-256 files, and tests the extracted VST3.
`./release/package-source.ps1 -JuceDirectory <JUCE-9.0.4>` packages all Git-tracked
project files plus the unmodified official JUCE checkout, without either .git
directory, logs, private history or build outputs.

Distribute the complete Corresponding Source alongside binaries, and update
release/SOURCE-CODE.txt and release notes for your own modified distribution.
See LICENSE.txt sections 5, 6 and 13 and THIRD_PARTY.md for the obligations.
Build success is not a promise of bit-for-bit reproducibility across toolchains.
