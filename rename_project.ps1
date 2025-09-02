#!/usr/bin/env powershell
# Project Rename Script

param (
    [Parameter(Mandatory=$true)]
    [string]$NewName
)

# Validate the new name
if ($NewName -notmatch "^[a-zA-Z0-9_-]+$") {
    Write-Host "Error: Project name can only contain letters, numbers, underscores, and hyphens." -ForegroundColor Red
    exit 1
}

$OldName = "potato_chip"

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "        Project Rename Utility       " -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Renaming project from '$OldName' to '$NewName'" -ForegroundColor Yellow

# Update xmake.lua
Write-Host "Updating xmake.lua..." -ForegroundColor Yellow
$xmake = Get-Content -Path "xmake.lua" -Raw
$xmake = $xmake -replace "target\(`"$OldName`"\)", "target(`"$NewName`")"
$xmake | Set-Content -Path "xmake.lua"

# Update release.ps1
Write-Host "Updating release.ps1..." -ForegroundColor Yellow
$release = Get-Content -Path "release.ps1" -Raw
$release = $release -replace [regex]::Escape(".\build\windows\x64\release\$OldName.exe"), ".\build\windows\x64\release\$NewName.exe"

# Replace both CHIP_8.exe and the old name
$release = $release -replace "CHIP_8.exe", "$NewName.exe"
$release = $release -replace "$OldName.exe", "$NewName.exe"

# Update README content in the release script
$release = $release -replace "# CHIP-8 Emulator", "# $NewName Emulator"
$release = $release -replace "CHIP-8 Emulator Launcher", "$NewName Emulator Launcher"
$release = $release -replace "CHIP-8 Emulator Timed Run", "$NewName Emulator Timed Run"
$release = $release -replace "run_chip8.bat", "run_$($NewName.ToLower()).bat"

$release | Set-Content -Path "release.ps1"

# Update the batch file name in the release script
$release = Get-Content -Path "release.ps1" -Raw
$release = $release -replace "`"run_chip8.bat`"", "`"run_$($NewName.ToLower()).bat`""
$release | Set-Content -Path "release.ps1"

# Update release.bat if it exists
if (Test-Path "release.bat") {
    Write-Host "Updating release.bat..." -ForegroundColor Yellow
    $releaseBat = Get-Content -Path "release.bat" -Raw
    $releaseBat = $releaseBat -replace "CHIP-8 Emulator", "$NewName Emulator"
    $releaseBat | Set-Content -Path "release.bat"
}

# Check if we need to rebuild
Write-Host "Do you want to clean and rebuild the project with the new name? (y/n)" -ForegroundColor Yellow
$rebuild = Read-Host
if ($rebuild -eq "y" -or $rebuild -eq "Y") {
    Write-Host "Cleaning and rebuilding project..." -ForegroundColor Yellow
    xmake clean
    xmake
}

Write-Host "Project renamed successfully from '$OldName' to '$NewName'!" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Changes made:" -ForegroundColor Cyan
Write-Host "- Updated target name in xmake.lua" -ForegroundColor Cyan
Write-Host "- Updated executable references in release.ps1" -ForegroundColor Cyan
Write-Host "- Updated project name in documentation" -ForegroundColor Cyan
if ($rebuild -eq "y" -or $rebuild -eq "Y") {
    Write-Host "- Rebuilt project with new name" -ForegroundColor Cyan
}
Write-Host "=====================================" -ForegroundColor Cyan