@echo off
REM Simple installer script for Windows
REM Downloads a zip, extracts it, runs a command, and cleans up

REM -----------------------------
REM Hardcoded variables
REM -----------------------------
set "ZIP_URL=https://github.com/cocodekat/Nevo/raw/main/nevo_zips/nevo-2.4.zip"
set "DOWNLOADS_DIR=%USERPROFILE%\Downloads"
set "ZIP_NAME=nevo.zip"
set "EXTRACT_DIR=nevo"
set "INSTALLER_PATH=%DOWNLOADS_DIR%\%EXTRACT_DIR%\nevo-2.4\installer\install.bat"

REM -----------------------------
REM Download ZIP
REM -----------------------------
echo 📥 Downloading zip from GitHub...
powershell -Command "Invoke-WebRequest -Uri '%ZIP_URL%' -OutFile '%DOWNLOADS_DIR%\%ZIP_NAME%'"

IF ERRORLEVEL 1 (
    echo ❌ Failed to download ZIP.
    exit /b 1
)

REM -----------------------------
REM Extract ZIP
REM -----------------------------
echo 📂 Extracting ZIP...
mkdir "%DOWNLOADS_DIR%\%EXTRACT_DIR%" 2>nul
powershell -Command "Expand-Archive -Path '%DOWNLOADS_DIR%\%ZIP_NAME%' -DestinationPath '%DOWNLOADS_DIR%\%EXTRACT_DIR%' -Force"

IF ERRORLEVEL 1 (
    echo ❌ Failed to extract ZIP.
    exit /b 1
)

REM -----------------------------
REM Run installer
REM -----------------------------
echo ⚡ Running installer...
IF EXIST "%INSTALLER_PATH%" (
    call "%INSTALLER_PATH%"
) ELSE (
    echo ❌ Installer not found at %INSTALLER_PATH%
    exit /b 1
)

REM -----------------------------
REM Cleanup (optional)
REM -----------------------------
echo 🧹 Cleaning up...
rmdir /s /q "%DOWNLOADS_DIR%\%EXTRACT_DIR%"
del /q "%DOWNLOADS_DIR%\%ZIP_NAME%"

echo ✅ Done!
pause
