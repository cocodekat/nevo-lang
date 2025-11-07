@echo off
REM Simple installer script for Windows: downloads a zip, extracts it, runs a command, and cleans up

REM -----------------------------
REM Hardcoded variables
REM -----------------------------
SET "ZIP_URL=https://github.com/cocodekat/Nevo/raw/main/nevo_zips/nevo-2.3.zip"
SET "DOWNLOADS_DIR=%USERPROFILE%\Downloads"
SET "ZIP_NAME=nevo.zip"
SET "EXTRACT_DIR=nevo"
SET "INSTALLER_PATH=%DOWNLOADS_DIR%\%EXTRACT_DIR%\nevo-2.3\installer\install.bat"

REM -----------------------------
REM Download ZIP
REM -----------------------------
echo 📥 Downloading zip from GitHub...
powershell -Command "Invoke-WebRequest -Uri '%ZIP_URL%' -OutFile '%DOWNLOADS_DIR%\%ZIP_NAME%'"

IF %ERRORLEVEL% NEQ 0 (
    echo ❌ Failed to download ZIP.
    exit /b 1
)

REM -----------------------------
REM Extract ZIP
REM -----------------------------
echo 📂 Extracting ZIP...
mkdir "%DOWNLOADS_DIR%\%EXTRACT_DIR%"
powershell -Command "Expand-Archive -Path '%DOWNLOADS_DIR%\%ZIP_NAME%' -DestinationPath '%DOWNLOADS_DIR%\%EXTRACT_DIR%' -Force"

IF %ERRORLEVEL% NEQ 0 (
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
