@echo off
echo CHIP-8 Project Rename Utility
echo ============================
echo.

set /p NEW_NAME="Enter new project name: "
powershell -ExecutionPolicy Bypass -File rename_project.ps1 -NewName "%NEW_NAME%"
pause