# SPDX-License-Identifier: AGPL-3.0-only
param([string]$BuildDirectory = '', [string]$JuceDirectory = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$dependencyArguments = @()
if ($JuceDirectory) {
    $JuceDirectory = [IO.Path]::GetFullPath($JuceDirectory)
    if (-not (Test-Path -LiteralPath (Join-Path $JuceDirectory 'CMakeLists.txt'))) { throw 'Invalid JUCE source directory.' }
    $dependencyArguments += "-DJUCE_PATH=$JuceDirectory"
}
if (-not $BuildDirectory) { $BuildDirectory = Join-Path $projectRoot ('build-distribution-' + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
$BuildDirectory = [IO.Path]::GetFullPath($BuildDirectory)
if (Test-Path -LiteralPath (Join-Path $BuildDirectory 'CMakeCache.txt')) { throw 'Use a new build directory for a clean distribution build.' }
$vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'MSVC build tools not found; no software was installed.' }
$cmake = Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctest = Join-Path (Split-Path $cmake) 'ctest.exe'
$vsMajor = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion).Split('.')[0]
$generator = switch ($vsMajor) { '18' { 'Visual Studio 18 2026' } '17' { 'Visual Studio 17 2022' } default { throw 'Use Visual Studio 2022 or 2026.' } }
& $cmake -S $projectRoot -B $BuildDirectory -G $generator -A x64 @dependencyArguments -DYURO_BUILD_TESTS=ON '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>'
if ($LASTEXITCODE) { throw 'Clean configure failed.' }
& $cmake --build $BuildDirectory --config Release --target Yurodstvo_Standalone Yurodstvo_VST3 YurodstvoTests --parallel 4
if ($LASTEXITCODE) { throw 'Clean Release build failed.' }
& $ctest --test-dir $BuildDirectory -C Release --output-on-failure
if ($LASTEXITCODE) { throw 'Release technical checks failed.' }
& (Join-Path $PSScriptRoot 'package-windows.ps1') -BuildDirectory $BuildDirectory
