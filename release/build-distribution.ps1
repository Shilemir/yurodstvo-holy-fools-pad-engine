param([string]$BuildDirectory = '', [string]$JuceDirectory = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
if (-not $JuceDirectory) { $JuceDirectory = Join-Path (Split-Path $projectRoot -Parent) 'JUCE' }
$JuceDirectory = [IO.Path]::GetFullPath($JuceDirectory)
if (-not (Test-Path -LiteralPath (Join-Path $JuceDirectory 'CMakeLists.txt'))) { throw 'Set -JuceDirectory to your separately obtained JUCE 9.0.4 checkout.' }
if (-not $BuildDirectory) { $BuildDirectory = Join-Path $projectRoot ('build-distribution-' + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
$BuildDirectory = [IO.Path]::GetFullPath($BuildDirectory)
if (Test-Path -LiteralPath (Join-Path $BuildDirectory 'CMakeCache.txt')) { throw 'Use a new build directory for a clean distribution build.' }
$vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'MSVC build tools not found; no software was installed.' }
$cmake = Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctest = Join-Path (Split-Path $cmake) 'ctest.exe'
& $cmake -S $projectRoot -B $BuildDirectory -G 'Visual Studio 18 2026' -A x64 "-DJUCE_PATH=$JuceDirectory" -DYURO_BUILD_TESTS=ON '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>'
if ($LASTEXITCODE) { throw 'Clean configure failed.' }
& $cmake --build $BuildDirectory --config Release --target Yurodstvo_Standalone Yurodstvo_VST3 YurodstvoTests --parallel 4
if ($LASTEXITCODE) { throw 'Clean Release build failed.' }
& $ctest --test-dir $BuildDirectory -C Release --output-on-failure
if ($LASTEXITCODE) { throw 'Release technical checks failed.' }
& (Join-Path $PSScriptRoot 'package-windows.ps1') -BuildDirectory $BuildDirectory
