@echo off
echo ========================================
echo   Solar System 3D - Quick Setup
echo ========================================
echo.
echo This will install the required dependencies using vcpkg.
echo.
pause

powershell -ExecutionPolicy Bypass -File "%~dp0setup.ps1"

pause
