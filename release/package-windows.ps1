# SPDX-License-Identifier: AGPL-3.0-only
param([Parameter(Mandatory=$true)][string]$BuildDirectory)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$BuildDirectory = [IO.Path]::GetFullPath($BuildDirectory)
$cmakeText = [IO.File]::ReadAllText((Join-Path $projectRoot 'CMakeLists.txt'))
$version = [regex]::Match($cmakeText,'project\([^\r\n]*VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)').Groups[1].Value
if (-not $version) { throw 'Cannot determine the application version.' }
$name = "YURODSTVO - Holy Fool's Pad Engine"
$bundleName = "$name.vst3"
$bundle = Join-Path $BuildDirectory "Yurodstvo_artefacts\Release\VST3\$bundleName"
$binaryRelative = "Contents\x86_64-win\$bundleName"
$binary = Join-Path $bundle $binaryRelative
$info = [Diagnostics.FileVersionInfo]::GetVersionInfo($binary)
if ($info.ProductName -ne $name -or $info.ProductVersion -ne $version) { throw 'Name/version mismatch.' }
$bytes = [IO.File]::ReadAllBytes($binary)
if ([BitConverter]::ToUInt16($bytes,[BitConverter]::ToInt32($bytes,60)+4) -ne 0x8664) { throw 'Not an x64 PE binary.' }
$vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$toolset = Get-ChildItem -LiteralPath (Join-Path $vs 'VC\Tools\MSVC') -Directory | Sort-Object Name -Descending | Select-Object -First 1
$dumpbin = Join-Path $toolset.FullName 'bin\Hostx64\x64\dumpbin.exe'
$dependencies = & $dumpbin /DEPENDENTS $binary | Out-String
if ($LASTEXITCODE) { throw 'Dependency inspection failed.' }
if ($dependencies -match '(?i)\b(?:VCRUNTIME|MSVCP|MSVCR|CONCRT|ucrtbased)[^\s]*\.dll') { throw 'Unexpected external MSVC runtime dependency.' }
$destination = Join-Path $projectRoot "artifacts\distribution\$version"
$runId = [guid]::NewGuid().ToString('N').Substring(0,12)
$stage = Join-Path $destination "staging-$runId"
$licences = Join-Path $stage 'LICENSES'
New-Item -ItemType Directory -Force -Path $licences | Out-Null
Copy-Item -LiteralPath $bundle -Destination $stage -Recurse
$readme = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'README.txt')).Replace('@VERSION@',$version)
[IO.File]::WriteAllText((Join-Path $stage 'README.txt'),$readme)
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'DISTRIBUTION-STATUS.txt'),(Join-Path $projectRoot 'LICENSE.txt'),(Join-Path $projectRoot 'COPYRIGHT.md'),(Join-Path $projectRoot 'THIRD_PARTY.md'),(Join-Path $projectRoot 'assets\ILLUSTRATION-NOTICE.txt'),(Join-Path $projectRoot 'LICENSES\CC0-1.0.txt'),(Join-Path $PSScriptRoot 'SOURCE-CODE.txt') -Destination $licences
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'SOURCE-CODE.txt') -Destination $stage
$juce = [regex]::Match([IO.File]::ReadAllText((Join-Path $BuildDirectory 'CMakeCache.txt')),'(?m)^JUCE_PATH:PATH=(.+)$').Groups[1].Value.Trim()
$juce = [IO.Path]::GetFullPath($juce)
$notices = @{
    'RuslanDisplay-OFL.txt' = (Join-Path $projectRoot 'assets\fonts\OFL.txt')
    'JUCE-LICENSE.md' = (Join-Path $juce 'LICENSE.md')
    'JUCE-inventory.spdx.json' = (Join-Path $juce 'JUCE.spdx.json')
    'VST3-SDK-MIT.txt' = (Join-Path $juce 'modules\juce_audio_processors_headless\format_types\VST3_SDK\LICENSE.txt')
    'zlib.txt' = (Join-Path $juce 'modules\juce_core\zip\zlib\LICENSE')
    'HarfBuzz.txt' = (Join-Path $juce 'modules\juce_graphics\fonts\harfbuzz\COPYING')
    'SheenBidi-Apache-2.0.txt' = (Join-Path $juce 'modules\juce_graphics\unicode\sheenbidi\LICENSE')
    'LunaSVG.txt' = (Join-Path $juce 'modules\juce_graphics\drawables\lunasvg\LICENSE')
    'PlutoVG.txt' = (Join-Path $juce 'modules\juce_graphics\drawables\lunasvg\plutovg\LICENSE')
    'libpng.txt' = (Join-Path $juce 'modules\juce_graphics\image_formats\pnglib\LICENSE')
    'libwebp.txt' = (Join-Path $juce 'modules\juce_graphics\image_formats\libwebp\COPYING')
    'libwebp-PATENTS.txt' = (Join-Path $juce 'modules\juce_graphics\image_formats\libwebp\PATENTS')
    'FLAC.txt' = (Join-Path $juce 'modules\juce_audio_formats\codecs\flac\Flac Licence.txt')
    'Ogg.txt' = (Join-Path $juce 'modules\juce_audio_formats\codecs\ogg\COPYING')
    'Vorbis.txt' = (Join-Path $juce 'modules\juce_audio_formats\codecs\vorbis\COPYING')
    'Opus.txt' = (Join-Path $juce 'modules\juce_audio_formats\codecs\opus\opus\COPYING')
    'Opusfile.txt' = (Join-Path $juce 'modules\juce_audio_formats\codecs\opus\opusfile\COPYING')
    'Opusenc.txt' = (Join-Path $juce 'modules\juce_audio_formats\codecs\opus\libopusenc\COPYING')
}
foreach ($entry in $notices.GetEnumerator()) { Copy-Item -LiteralPath $entry.Value -Destination (Join-Path $licences $entry.Key) }
foreach ($sdkPart in @('base','pluginterfaces','public.sdk')) {
    Copy-Item -LiteralPath (Join-Path $juce "modules\juce_audio_processors_headless\format_types\VST3_SDK\$sdkPart\LICENSE.txt") -Destination (Join-Path $licences "VST3-$sdkPart-MIT.txt")
}
$presonusNotices = Get-ChildItem -LiteralPath (Join-Path $juce 'modules\juce_audio_processors_headless\format_types\pslextensions') -File | ForEach-Object {
    $text = [IO.File]::ReadAllText($_.FullName)
    $end = $text.IndexOf('#ifndef')
    if ($end -gt 0) { "File: $($_.Name)`r`n" + $text.Substring(0,$end) }
}
[IO.File]::WriteAllText((Join-Path $licences 'PreSonus-public-domain.txt'),($presonusNotices -join "`r`n"))
Get-ChildItem -LiteralPath (Join-Path $juce 'modules\juce_audio_formats\codecs\opus') -Recurse -File | Where-Object {
    $_.Name -match '^(PATENTS|LICENSE_PLEASE_READ|COPYRIGHT|AUTHORS)'
} | ForEach-Object {
    $noticeName = $_.FullName.Substring((Join-Path $juce 'modules\juce_audio_formats\codecs\opus').Length+1).Replace('\','-')
    Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $licences ("Opus-"+$noticeName))
}
$jpegReadme = [IO.File]::ReadAllText((Join-Path $juce 'modules\juce_graphics\image_formats\jpglib\README'))
$legal = [regex]::Match($jpegReadme,'(?ms)^LEGAL ISSUES\r?\n=+.*?(?=^REFERENCES\r?\n=+)').Value
if (-not $legal) { throw 'JPEG licence section not found.' }
[IO.File]::WriteAllText((Join-Path $licences 'Independent-JPEG-Group.txt'),$legal)
[IO.File]::WriteAllText((Join-Path $licences 'THIRD-PARTY-NOTICES.txt'),@'
This software is based in part on the work of the Independent JPEG Group.
SheenBidi copyright (C) 2014-2025 Muhammad Tayyab Akram; Apache License 2.0.
The accompanying licence files reproduce third-party notices from JUCE 9.0.4.
JUCE-inventory.spdx.json is the upstream framework inventory, not a claim that
all listed optional components are linked into this plugin. JUCE is used under
AGPL-3.0-only. Complete Corresponding Source, including JUCE, is available from
the same release as these binaries; see SOURCE-CODE.txt. The application is
AGPL-3.0-only. The font remains OFL-1.1; only the official PNG is dedicated under
CC0-1.0 to the extent of the owner's rights. See ILLUSTRATION-NOTICE.txt.
'@)
if (-not ('EmbeddedReleaseAsset' -as [type])) {
    Add-Type -TypeDefinition @'
public static class EmbeddedReleaseAsset {
    public static int Find(byte[] binary, byte[] asset) {
        for (int i = 0; i <= binary.Length - asset.Length; ++i) {
            if (binary[i] != asset[0]) continue;
            int j = 1;
            while (j < asset.Length && binary[i+j] == asset[j]) ++j;
            if (j == asset.Length) return i;
        }
        return -1;
    }
}
'@
}
foreach ($assetName in @('assets\yurodstvo_icon.png','assets\fonts\RuslanDisplay-Regular.ttf')) {
    $assetBytes = [IO.File]::ReadAllBytes((Join-Path $projectRoot $assetName))
    if ([EmbeddedReleaseAsset]::Find($bytes,$assetBytes) -lt 0) { throw "Asset is not embedded intact: $assetName" }
}
$bundleNotices = Join-Path $stage "$bundleName\Contents\Resources\Licenses"
New-Item -ItemType Directory -Force -Path $bundleNotices | Out-Null
Copy-Item -LiteralPath (Get-ChildItem -LiteralPath $licences -File | ForEach-Object FullName) -Destination $bundleNotices
if (Get-ChildItem -LiteralPath $stage -Recurse -File | Where-Object { $_.Extension -in '.pdb','.lib','.exp','.ilk','.obj','.cpp','.h','.ps1','.vcxproj' }) { throw 'Unexpected source/build/debug files in package.' }
function New-VerifiedArchive([string]$Source,[string]$Kind) {
    $zipName = "YURODSTVO-Holy-Fools-Pad-Engine-v$version-Windows-$Kind.zip"
    $zipPath = Join-Path $destination $zipName
    $temporaryZip = Join-Path $destination "candidate-$runId-$Kind.zip"
    Add-Type -AssemblyName System.IO.Compression,System.IO.Compression.FileSystem
    $zipStream = [IO.File]::Open($temporaryZip,[IO.FileMode]::CreateNew)
    $archive = [IO.Compression.ZipArchive]::new($zipStream,[IO.Compression.ZipArchiveMode]::Create,$false)
    try {
        foreach ($file in Get-ChildItem -LiteralPath $Source -Recurse -File | Sort-Object FullName) {
            if ($file.Extension -in '.pdb','.lib','.exp','.ilk','.obj','.cpp','.h','.ps1','.vcxproj') { throw 'Unexpected development file.' }
            $relative = $file.FullName.Substring($Source.Length+1).Replace('\','/')
            $entry = $archive.CreateEntry($relative,[IO.Compression.CompressionLevel]::Optimal)
            $out = $entry.Open(); $input = $file.OpenRead()
            try { $input.CopyTo($out) } finally { $input.Dispose(); $out.Dispose() }
        }
    } finally { $archive.Dispose(); $zipStream.Dispose() }
    $verify = Join-Path $destination "verify-$runId-$Kind"
    [IO.Compression.ZipFile]::ExtractToDirectory($temporaryZip,$verify)
    foreach ($file in Get-ChildItem -LiteralPath $Source -Recurse -File) {
        $copy = Join-Path $verify $file.FullName.Substring($Source.Length+1)
        if ((Get-FileHash -LiteralPath $file.FullName).Hash -ne (Get-FileHash -LiteralPath $copy).Hash) { throw 'ZIP roundtrip mismatch.' }
    }
    Move-Item -LiteralPath $temporaryZip -Destination $zipPath -Force
    $hash = (Get-FileHash -LiteralPath $zipPath -Algorithm SHA256).Hash
    [IO.File]::WriteAllText(($zipPath+'.sha256'),"$hash  $zipName`r`n")
    return [pscustomobject]@{Kind=$Kind;ZIP=$zipPath;SHA256=$hash;Extracted=$verify}
}
$vst = New-VerifiedArchive $stage 'VST3'
$standalone = Join-Path $BuildDirectory "Yurodstvo_artefacts\Release\Standalone\$name.exe"
$appInfo = [Diagnostics.FileVersionInfo]::GetVersionInfo($standalone)
if ($appInfo.ProductName -ne $name -or $appInfo.ProductVersion -ne $version) { throw 'Standalone metadata mismatch.' }
$appBytes = [IO.File]::ReadAllBytes($standalone)
if ([BitConverter]::ToUInt16($appBytes,[BitConverter]::ToInt32($appBytes,60)+4) -ne 0x8664) { throw 'Standalone is not x64.' }
$appDependencies = & $dumpbin /DEPENDENTS $standalone | Out-String
if ($LASTEXITCODE -or $appDependencies -match '(?i)\b(?:VCRUNTIME|MSVCP|MSVCR|CONCRT|ucrtbased)[^\s]*\.dll') { throw 'Standalone runtime inspection failed.' }
foreach ($assetName in @('assets\yurodstvo_icon.png','assets\fonts\RuslanDisplay-Regular.ttf')) {
    if ([EmbeddedReleaseAsset]::Find($appBytes,[IO.File]::ReadAllBytes((Join-Path $projectRoot $assetName))) -lt 0) { throw "Standalone missing $assetName" }
}
$appStage = Join-Path $destination "standalone-$runId"
New-Item -ItemType Directory -Force -Path $appStage | Out-Null
Copy-Item -LiteralPath $standalone -Destination $appStage
Copy-Item -LiteralPath $licences -Destination $appStage -Recurse
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'SOURCE-CODE.txt') -Destination $appStage
$appReadme = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'README-Standalone.txt')).Replace('@VERSION@',$version)
[IO.File]::WriteAllText((Join-Path $appStage 'README.txt'),$appReadme)
$app = New-VerifiedArchive $appStage 'Standalone'
$test = Join-Path $BuildDirectory 'YurodstvoTests_artefacts\Release\YurodstvoTests.exe'
$baseline = Join-Path $projectRoot 'artifacts\0.6\VST3\YURODSTVO - Holy Fool''s Pad Engine.vst3'
$arguments = @((Join-Path $destination 'verification'),(Join-Path $vst.Extracted $bundleName))
if (Test-Path -LiteralPath $baseline) { $arguments += $baseline }
& $test @arguments > (Join-Path $destination 'Extracted-ZIP-tests.log') 2>&1
if ($LASTEXITCODE) { throw 'Extracted VST3 failed technical checks.' }
[IO.File]::WriteAllText((Join-Path $destination 'Dependencies-VST3.txt'),$dependencies)
[IO.File]::WriteAllText((Join-Path $destination 'Dependencies-Standalone.txt'),$appDependencies)
@($vst,$app) | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $destination 'packages.json') -Encoding UTF8
$vst,$app | Format-List
