@echo off

set TARGET_DIR=C:\nevo

set "ZIP_PATH=%TARGET_DIR%\tcc.zip"
mkdir C:\nevo\Libraries\Images
mkdir C:\nevo\Modes

set URL=https://download.savannah.gnu.org/releases/tinycc/tcc-0.9.26-win64-bin.zip

echo Installing tcc... (This might take a moment)

powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri '%URL%' -OutFile '%ZIP_PATH%'"

:: Extract the zip
set "EXTRACT_PATH=%TARGET_DIR%\tcc"
echo Extracting TCC...
powershell -Command "Expand-Archive -Path '%ZIP_PATH%' -DestinationPath '%EXTRACT_PATH%' -Force"

:: Delete the zip
del "%ZIP_PATH%"

REM Make sure the working directory is the folder where this script is
cd /d "%~dp0"

REM Go one directory up from script location
cd ..

REM Optional: Show current path
echo Current directory: %CD%

REM Compile C files from current dir (nevo-2\)
C:\nevo\tcc\tcc\tcc.exe nevo.c -o C:\nevo\nevo.exe
C:\nevo\tcc\tcc\tcc.exe modes\n.c auto_var.c ban_list.c -o C:\nevo\Modes\n.exe

REM Move header files into C:\nevo\libraries\
move /y Libraries C:\nevo >nul 2>&1

REM ── Add C:\nevo to PATH permanently if not already there
set "REG_KEY=HKCU\Environment"
for /f "tokens=2*" %%A in ('reg query "%REG_KEY%" /v Path 2^>nul ^| findstr /i "%TARGET_DIR%"') do set "EXISTS=%%B"
if not defined EXISTS (
    echo Adding %TARGET_DIR% to PATH...
    setx PATH "%TARGET_DIR%;%PATH%"
    echo ✅ Added %TARGET_DIR% to PATH. Restart your terminal or log off/log on for changes to take effect.
) else (
    echo %TARGET_DIR% is already in PATH.
)

echo done! Everything you need is inside of C:\nevo Compile files using nevo.exe -flag input_file"
pause