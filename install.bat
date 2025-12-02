@echo off
echo ------------------------------------------
echo   Determa Compiler Installer (Windows)
echo ------------------------------------------
echo.

REM 1. Build the latest version
echo [1/3] Building latest version...
call build.bat
if %errorlevel% neq 0 (
    echo [ERROR] Build failed. Installation aborted.
    pause
    exit /b %errorlevel%
)

REM 2. Get absolute path to bin directory
set "BIN_PATH=%~dp0bin"
echo [2/3] Registering path: %BIN_PATH%

REM 3. Check if already in PATH (Simple check)
echo %PATH% | find /i "%BIN_PATH%" > nul
if %errorlevel% equ 0 (
    echo [SKIP] Determa is already in your PATH.
) else (
    echo [3/3] Adding to User PATH environment variable...
    echo.
    echo NOTE: This uses 'setx' to modify your user PATH.
    echo It will NOT affect currently open terminals.
    echo You must restart your terminal after this completes.
    echo.
    
    REM Append to PATH safely
    setx PATH "%PATH%;%BIN_PATH%"
    
    if %errorlevel% neq 0 (
        echo [ERROR] Failed to set PATH. Try running as Administrator.
    ) else (
        echo [SUCCESS] Determa installed successfully!
    )
)

echo.
echo ------------------------------------------
echo Installation Complete.
echo Please RESTART your terminal (CMD/PowerShell).
echo Then type 'determa' to test.
echo ------------------------------------------
pause