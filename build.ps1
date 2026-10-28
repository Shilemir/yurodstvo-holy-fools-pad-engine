# SPDX-License-Identifier: AGPL-3.0-only
$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$version = [regex]::Match([IO.File]::ReadAllText((Join-Path $projectRoot 'CMakeLists.txt')), 'project\([^\r\n]*VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)').Groups[1].Value
if (-not $version) { throw 'Cannot determine application version.' }
$vswherePath = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vsRoot = & $vswherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsRoot) { throw 'Visual Studio C++ tools were not found.' }
$cmakePath = Join-Path $vsRoot 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctestPath = Join-Path (Split-Path $cmakePath) 'ctest.exe'
$buildDir = Join-Path $projectRoot 'build'
& $cmakePath -S $projectRoot -B $buildDir -G 'Visual Studio 18 2026' -A x64
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
& $cmakePath --build $buildDir --config Release --target Yurodstvo_Standalone Yurodstvo_VST3 YurodstvoTests --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
# Check actual Windows resources, which may otherwise remain cached after a rename.
$productName = "YURODSTVO - Holy Fool's Pad Engine"
foreach ($binaryPath in @(
    (Join-Path $buildDir "Yurodstvo_artefacts\Release\Standalone\$productName.exe"),
    (Join-Path $buildDir "Yurodstvo_artefacts\Release\VST3\$productName.vst3\Contents\x86_64-win\$productName.vst3"))) {
    $versionInfo = [Diagnostics.FileVersionInfo]::GetVersionInfo($binaryPath)
    if ($versionInfo.ProductName -ne $productName -or $versionInfo.ProductVersion -ne $version) {
        throw "Outdated Windows metadata: $binaryPath"
    }
}
& $ctestPath --test-dir $buildDir -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Technical tests failed.' }
$artifactsDir = Join-Path $projectRoot "artifacts\$version"
New-Item -ItemType Directory -Force -Path (Join-Path $artifactsDir 'Standalone'),(Join-Path $artifactsDir 'VST3') | Out-Null
Copy-Item -LiteralPath (Join-Path $buildDir "Yurodstvo_artefacts\Release\Standalone\YURODSTVO - Holy Fool's Pad Engine.exe") -Destination (Join-Path $artifactsDir 'Standalone') -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'assets\fonts\OFL.txt') -Destination (Join-Path $artifactsDir 'Standalone\RuslanDisplay-OFL.txt') -Force
Copy-Item -LiteralPath (Join-Path $buildDir "Yurodstvo_artefacts\Release\VST3\YURODSTVO - Holy Fool's Pad Engine.vst3") -Destination (Join-Path $artifactsDir 'VST3') -Recurse -Force
Copy-Item -LiteralPath (Join-Path $buildDir 'Testing\Temporary\LastTest.log') -Destination (Join-Path $artifactsDir 'TechnicalTests.log') -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'artifacts\YURODSTVO-interface.png') -Destination $artifactsDir -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'artifacts\EQ-response-0.8.csv') -Destination $artifactsDir -Force
$nominalCsv = Join-Path $projectRoot 'artifacts\Nominal-levels-0.8.csv'
if (Test-Path -LiteralPath $nominalCsv) { Copy-Item -LiteralPath $nominalCsv -Destination $artifactsDir -Force }
Copy-Item -LiteralPath (Join-Path $projectRoot 'README.md'),(Join-Path $projectRoot 'TEST_REPORT.md') -Destination $artifactsDir -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'DSP-REFINEMENT.md'),(Join-Path $projectRoot 'artifacts\Cathedral-delay-impulses.csv') -Destination $artifactsDir -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'SACRED-MODULES.md'),(Join-Path $projectRoot 'artifacts\Sacred-stress.txt') -Destination $artifactsDir -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'artifacts\Polyphony-button-states.png') -Destination $artifactsDir -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'INSPIRATION.md'),(Join-Path $projectRoot 'artifacts\Inspiration-stress.txt'),(Join-Path $projectRoot 'artifacts\Release-glow-states.png') -Destination $artifactsDir -Force
Write-Output "Ready: $artifactsDir"
