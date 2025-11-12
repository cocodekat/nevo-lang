@echo off

set TARGET_DIR=C:\nevo

set "ZIP_PATH=%TARGET_DIR%\tcc.zip"
mkdir C:\nevo\libraries\Images
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

REM Go one directory up from script location (installer\ -> nevo-2\)
cd ..

REM Optional: Show current path
echo Current directory: %CD%

REM Compile C files from current dir (nevo-2\)
C:\nevo\tcc\tcc\tcc.exe nevo.c -o C:\nevo\nevo.exe
C:\nevo\tcc\tcc\tcc.exe modes\n.c auto_var.c ban_list.c -o C:\nevo\Modes\n.exe

REM Move header files into C:\nevo\libraries\
move /y arradd.h C:\nevo\libraries\ >nul 2>&1
move /y h1.h C:\nevo\libraries\ >nul 2>&1
move /y sha256.h C:\nevo\libraries\ >nul 2>&1

move /y better_ops.h C:\nevo\libraries\ >nul 2>&1
move /y bool.h C:\nevo\libraries\ >nul 2>&1
move /y unless.h C:\nevo\libraries\ >nul 2>&1

move /y npxm.h C:\nevo\libraries\ >nul 2>&1
move /y stb_image_write.h C:\nevo\libraries\ >nul 2>&1
move /y stb_image.h C:\nevo\libraries\ >nul 2>&1

echo done! Everything you need is inside of C:\nevo Compile files using C:\nevo\nevo -flag input_file"
pause