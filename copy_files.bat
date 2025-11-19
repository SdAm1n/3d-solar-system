@echo off
REM Post-build script to copy required shader files

echo Copying shader files...
copy /Y "planet.vs" "x64\Release\" >nul 2>&1
copy /Y "planet.fs" "x64\Release\" >nul 2>&1
copy /Y "orbit.vs" "x64\Release\" >nul 2>&1
copy /Y "orbit.fs" "x64\Release\" >nul 2>&1
copy /Y "skybox.vs" "x64\Release\" >nul 2>&1
copy /Y "skybox.fs" "x64\Release\" >nul 2>&1

copy /Y "planet.vs" "x64\Debug\" >nul 2>&1
copy /Y "planet.fs" "x64\Debug\" >nul 2>&1
copy /Y "orbit.vs" "x64\Debug\" >nul 2>&1
copy /Y "orbit.fs" "x64\Debug\" >nul 2>&1
copy /Y "skybox.vs" "x64\Debug\" >nul 2>&1
copy /Y "skybox.fs" "x64\Debug\" >nul 2>&1

echo.
echo All shader files copied successfully!
echo   - planet.vs/fs
echo   - orbit.vs/fs
echo   - skybox.vs/fs
echo.
echo NOTE: If you get DLL errors when running, ensure vcpkg integration is active.
echo Run: vcpkg integrate install
echo.
pause
