#!/usr/bin/env powershell
# CHIP-8 Emulator Release Script

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "   CHIP-8 Emulator Release Builder   " -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan

# Create release directory if it doesn't exist
$releaseDir = ".\release"
if (-not (Test-Path $releaseDir)) {
    Write-Host "Creating release directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $releaseDir | Out-Null
} else {
    Write-Host "Cleaning existing release directory..." -ForegroundColor Yellow
    Get-ChildItem -Path $releaseDir -Recurse | Remove-Item -Force -Recurse
}

# Create roms directory inside release
$romsDir = "$releaseDir\roms"
if (-not (Test-Path $romsDir)) {
    Write-Host "Creating roms directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $romsDir | Out-Null
}

# Build the release version using xmake
Write-Host "Building release version..." -ForegroundColor Yellow
xmake clean
# Configure xmake to use release mode
xmake f -m release
# Build in release mode with optimizations
xmake -v

# Copy the executable to release directory
Write-Host "Copying executable..." -ForegroundColor Yellow
Copy-Item ".\build\windows\x64\release\potato_chip.exe" -Destination $releaseDir

# Copy the beep sound file
Write-Host "Copying sound files..." -ForegroundColor Yellow
Copy-Item ".\beep.wav" -Destination $releaseDir

# Copy ROM files
Write-Host "Copying ROM files..." -ForegroundColor Yellow
Copy-Item ".\roms\*.ch8" -Destination $romsDir -Recurse

# Create README.md
Write-Host "Creating README file..." -ForegroundColor Yellow
@"
# CHIP-8 Emulator

A CHIP-8 emulator written in C using raylib for graphics and audio.

## Usage

Run the emulator by double-clicking the `run_chip8.bat` file or by using the command:

```
potato_chip.exe roms/[ROM_FILE].ch8
```

Replace `[ROM_FILE]` with the name of the ROM you want to run.

### Timed Run Mode

You can run a ROM for exactly 3 seconds (useful for demos or testing) using:

```
potato_chip.exe roms/[ROM_FILE].ch8 --timed
```

Or simply use the `timed_run.bat` batch file.

### Sound Support

The emulator looks for a `beep.wav` file in the following locations:
- Current directory where the emulator is run from
- Directory containing the executable
- Parent directory of the executable

Make sure to keep the `beep.wav` file in the same directory as the executable for sound to work properly.

## Controls

The CHIP-8 keypad is mapped to the following keys:

```
CHIP-8 Keypad:       Keyboard Mapping:
1 2 3 C              1 2 3 4
4 5 6 D              Q W E R
7 8 9 E              A S D F
A 0 B F              Z X C V
```

Press `ESC` to exit the emulator.

## Included ROMs

The emulator comes with the following ROMs:
- bullet.ch8

## Build Information
- Built with xmake in Release mode
- Release Date: $(Get-Date -Format "yyyy-MM-dd")
- Version: 1.0.0

## Credits

Created by Brendan
"@ | Set-Content -Path "$releaseDir\README.md"

# Create batch file for easy running
Write-Host "Creating batch launcher..." -ForegroundColor Yellow
@"
@echo off
echo CHIP-8 Emulator Launcher
echo ========================
echo.

REM Check if a ROM file was provided
if "%~1"=="" (
    echo No ROM file specified. Available ROMs:
    echo.
    dir /b roms\*.ch8
    echo.
    set /p ROM_FILE="Enter ROM filename (from roms folder): "
    potato_chip.exe "roms\%ROM_FILE%"
) else (
    potato_chip.exe "%~1"
)
pause
"@ | Set-Content -Path "$releaseDir\run_chip8.bat"

# Create shortcut to a specific ROM as an example
Write-Host "Creating ROM shortcuts..." -ForegroundColor Yellow
@"
@echo off
potato_chip.exe "roms\bullet.ch8"
"@ | Set-Content -Path "$releaseDir\play_bullet.bat"

# Create timed run batch file
Write-Host "Creating timed run launcher..." -ForegroundColor Yellow
@"
@echo off
echo CHIP-8 Emulator Timed Run (3 seconds)
echo ====================================
echo.

REM Check if a ROM file was provided
if "%~1"=="" (
    echo No ROM file specified. Available ROMs:
    echo.
    dir /b roms\*.ch8
    echo.
    set /p ROM_FILE="Enter ROM filename (from roms folder): "
    potato_chip.exe "roms\%ROM_FILE%" --timed
) else (
    potato_chip.exe "%~1" --timed
)
pause
"@ | Set-Content -Path "$releaseDir\timed_run.bat"

# Create a version info file
Write-Host "Creating version info..." -ForegroundColor Yellow
@"
{
  "name": "CHIP-8 Emulator",
  "version": "1.0.0",
  "build_date": "$(Get-Date -Format "yyyy-MM-dd")",
  "build_mode": "release",
  "description": "A CHIP-8 emulator written in C using raylib",
  "author": "Brendan"
}
"@ | Set-Content -Path "$releaseDir\version.json"

# Create a simple shell script for Linux users
Write-Host "Creating shell script for Linux users..." -ForegroundColor Yellow
@"
#!/bin/bash
# CHIP-8 Emulator Launcher for Linux

if [ -z "\$1" ]; then
    echo "No ROM file specified. Available ROMs:"
    echo
    ls -1 roms/*.ch8 2>/dev/null
    echo
    read -p "Enter ROM filename (from roms folder): " ROM_FILE
    ./potato_chip "roms/\$ROM_FILE"
else
    ./potato_chip "\$1"
fi
"@ | Set-Content -Path "$releaseDir\run_chip8.sh" -NoNewline

Write-Host "Release package created successfully!" -ForegroundColor Green
Write-Host "Location: $((Get-Item $releaseDir).FullName)" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Release Summary:" -ForegroundColor Cyan
Write-Host "- Build Mode: Release (optimized)" -ForegroundColor Cyan
Write-Host "- Executable: potato_chip.exe" -ForegroundColor Cyan
Write-Host "- ROM Files: $(Get-ChildItem -Path $romsDir -Filter *.ch8 | Measure-Object | Select-Object -ExpandProperty Count)" -ForegroundColor Cyan
Write-Host "- Created Date: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan