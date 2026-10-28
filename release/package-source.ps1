# SPDX-License-Identifier: AGPL-3.0-only
param([Parameter(Mandatory=$true)][string]$JuceDirectory)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem
$root = Split-Path $PSScriptRoot -Parent
$version = [regex]::Match([IO.File]::ReadAllText((Join-Path $root 'CMakeLists.txt')), 'project\([^\r\n]*VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)').Groups[1].Value
if (-not $version) { throw 'Cannot determine application version.' }
$JuceDirectory = [IO.Path]::GetFullPath($JuceDirectory)
$commit = & git -C $root rev-parse HEAD
if ($LASTEXITCODE) { throw 'Project Git revision unavailable.' }
if (& git -C $root status --porcelain --untracked-files=no) { throw 'Commit project changes before packaging source.' }
$juceCommit = & git -C $JuceDirectory rev-parse HEAD
if ($LASTEXITCODE -or $juceCommit -ne '3fb5cbc0340758596dac39f7c8de47d562b4896e') { throw 'Use the exact upstream JUCE 9.0.4 checkout.' }
if (& git -C $JuceDirectory status --porcelain --untracked-files=no) { throw 'JUCE must be unmodified.' }
$output = Join-Path $root "artifacts\distribution\$version"
New-Item -ItemType Directory -Path $output -Force | Out-Null
$stage = Join-Path $output ('source-'+[guid]::NewGuid().ToString('N'))
$projectArchive = $stage+'.zip'
& git -C $root archive --format=zip --output=$projectArchive HEAD
if ($LASTEXITCODE) { throw 'Project source export failed.' }
[IO.Compression.ZipFile]::ExtractToDirectory($projectArchive,$stage)
$juceArchive = $stage+'-JUCE.zip'
& git -C $JuceDirectory archive --format=zip --output=$juceArchive HEAD
if ($LASTEXITCODE) { throw 'JUCE source export failed.' }
[IO.Compression.ZipFile]::ExtractToDirectory($juceArchive,(Join-Path $stage 'dependencies\JUCE'))
[IO.File]::WriteAllText((Join-Path $stage 'SOURCE-MANIFEST.txt'),"Application: $commit`nSource tag: v$version`nJUCE: $juceCommit (9.0.4)`nApplication version: $version`nLicence: AGPL-3.0-only; see COPYRIGHT.md and THIRD_PARTY.md for exceptions.`n")
if (Get-ChildItem -LiteralPath $stage -Force -Recurse | Where-Object { $_.Name -eq '.git' -or $_.FullName -match '[\\/]assets[\\/]reference[\\/]' }) { throw 'Private content in source stage.' }
$zip = Join-Path $output "YURODSTVO-v$version-Corresponding-Source.zip"
if (Test-Path -LiteralPath $zip) { throw 'Source archive already exists; preserve previous packages.' }
$stream = [IO.File]::Open($zip,[IO.FileMode]::CreateNew)
$archive = [IO.Compression.ZipArchive]::new($stream,[IO.Compression.ZipArchiveMode]::Create,$false)
try {
    foreach ($file in Get-ChildItem -LiteralPath $stage -Recurse -File | Sort-Object FullName) {
        $relative = $file.FullName.Substring($stage.Length+1).Replace('\','/')
        $entry = $archive.CreateEntry($relative,[IO.Compression.CompressionLevel]::Optimal)
        $input = $file.OpenRead()
        $outputStream = $entry.Open()
        try { $input.CopyTo($outputStream) } finally { $input.Dispose(); $outputStream.Dispose() }
    }
} finally { $archive.Dispose(); $stream.Dispose() }
$check = [IO.Compression.ZipFile]::OpenRead($zip)
try {
    foreach ($entry in $check.Entries) {
        if ($entry.FullName.Contains('\')) { throw 'Nonportable ZIP path.' }
        $input = $entry.Open()
        $sha = [Security.Cryptography.SHA256]::Create()
        try { $entryHash = [BitConverter]::ToString($sha.ComputeHash($input)).Replace('-','') }
        finally { $input.Dispose(); $sha.Dispose() }
        if ($entryHash -ne (Get-FileHash -LiteralPath (Join-Path $stage $entry.FullName) -Algorithm SHA256).Hash) {
            throw 'Source ZIP roundtrip mismatch.'
        }
    }
} finally { $check.Dispose() }
$hash = (Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash
[IO.File]::WriteAllText(($zip+'.sha256'),"$hash  $([IO.Path]::GetFileName($zip))`r`n")
Write-Output $zip
Write-Output $hash

$checksumLines = @(Get-ChildItem -LiteralPath $output -Filter "YURODSTVO-v$version-*.zip" | Sort-Object Name | ForEach-Object { (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash + '  ' + $_.Name })
[IO.File]::WriteAllText((Join-Path $output 'SHA256SUMS.txt'),($checksumLines -join "`r`n")+"`r`n")
