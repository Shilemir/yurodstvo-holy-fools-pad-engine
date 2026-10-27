# SPDX-License-Identifier: AGPL-3.0-only
param([Parameter(Mandatory=$true)][string]$JuceDirectory)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$JuceDirectory = [IO.Path]::GetFullPath($JuceDirectory)
$commit = & git -C $root rev-parse HEAD
if ($LASTEXITCODE) { throw 'Project Git revision unavailable.' }
if (& git -C $root status --porcelain --untracked-files=no) { throw 'Commit project changes before packaging source.' }
$juceCommit = & git -C $JuceDirectory rev-parse HEAD
if ($LASTEXITCODE -or $juceCommit -ne '3fb5cbc0340758596dac39f7c8de47d562b4896e') { throw 'Use the exact upstream JUCE 9.0.4 checkout.' }
if (& git -C $JuceDirectory status --porcelain --untracked-files=no) { throw 'JUCE must be unmodified.' }
$output = Join-Path $root 'artifacts\distribution\0.8.0-agpl'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$stage = Join-Path $output ('source-'+[guid]::NewGuid().ToString('N'))
$projectArchive = $stage+'.zip'
& git -C $root archive --format=zip --output=$projectArchive HEAD
if ($LASTEXITCODE) { throw 'Project source export failed.' }
Expand-Archive -LiteralPath $projectArchive -DestinationPath $stage
$juceArchive = $stage+'-JUCE.zip'
& git -C $JuceDirectory archive --format=zip --output=$juceArchive HEAD
if ($LASTEXITCODE) { throw 'JUCE source export failed.' }
Expand-Archive -LiteralPath $juceArchive -DestinationPath (Join-Path $stage 'dependencies\JUCE')
[IO.File]::WriteAllText((Join-Path $stage 'SOURCE-MANIFEST.txt'),"Application: $commit`nSource tag: v0.8.0-agpl`nJUCE: $juceCommit (9.0.4)`nApplication version: 0.8.0`nLicence: AGPL-3.0-only; see COPYRIGHT.md and THIRD_PARTY.md for exceptions.`n")
if (Get-ChildItem -LiteralPath $stage -Force -Recurse | Where-Object { $_.Name -eq '.git' -or $_.FullName -match '[\\/]assets[\\/]reference[\\/]' }) { throw 'Private content in source stage.' }
Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip = Join-Path $output 'YURODSTVO-Holy-Fools-Pad-Engine-v0.8.0-AGPL-Corresponding-Source.zip'
if (Test-Path -LiteralPath $zip) { throw 'Source archive already exists; preserve previous packages.' }
[IO.Compression.ZipFile]::CreateFromDirectory($stage,$zip,[IO.Compression.CompressionLevel]::Optimal,$false)
$hash = (Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash
[IO.File]::WriteAllText(($zip+'.sha256'),"$hash  $([IO.Path]::GetFileName($zip))`r`n")
Write-Output $zip
Write-Output $hash
