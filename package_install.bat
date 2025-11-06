@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

:: -----------------------------------------------
:: Variables
:: -----------------------------------------------
set "GITHUB_URL=https://github.com/cocodekat/Nevo/raw/main/package.py"
set "INSTALL_DIR=%USERPROFILE%\package"
set "PORTABLE_PYTHON_DIR=%INSTALL_DIR%\python"
set "SYSTEM_PYTHON=C:\python-nevo\python.exe"

mkdir "%INSTALL_DIR%" 2>nul

echo 📥 Downloading package.py from GitHub …
powershell -Command "Invoke-WebRequest -Uri '%GITHUB_URL%' -OutFile '%INSTALL_DIR%\package.py'"
if not exist "%INSTALL_DIR%\package.py" (
    echo ❌ Failed to download package.py
    exit /b 1
)
echo ✅ Downloaded Python file: %INSTALL_DIR%\package.py

:: -----------------------------------------------
:: Download portable Python (optional fallback)
:: -----------------------------------------------
set "PYTHON_ZIP=%INSTALL_DIR%\python-3.12.0-embed.zip"
echo 📥 Downloading portable Python …
powershell -Command "Invoke-WebRequest -Uri 'https://www.python.org/ftp/python/3.12.0/python-3.12.0-embed-amd64.zip' -OutFile '%PYTHON_ZIP%'"
mkdir "%PORTABLE_PYTHON_DIR%" 2>nul
powershell -Command "Expand-Archive -Path '%PYTHON_ZIP%' -DestinationPath '%PORTABLE_PYTHON_DIR%' -Force"
del "%PYTHON_ZIP%"

:: -----------------------------------------------
:: Detect Python executable
:: -----------------------------------------------
if exist "%SYSTEM_PYTHON%" (
    set "PYTHON_BIN=%SYSTEM_PYTHON%"
) else (
    if exist "%PORTABLE_PYTHON_DIR%\python.exe" (
        set "PYTHON_BIN=%PORTABLE_PYTHON_DIR%\python.exe"
    ) else (
        for %%p in (python python3) do (
            where %%p >nul 2>nul && set "PYTHON_BIN=%%p"
        )
    )
)

if not exist "%PYTHON_BIN%" (
    echo ❌ No Python found!
    exit /b 1
)

echo ✅ Using Python: %PYTHON_BIN%

:: -----------------------------------------------
:: Install dependencies using system Python
:: -----------------------------------------------
"%PYTHON_BIN%" -m ensurepip --upgrade
"%PYTHON_BIN%" -m pip install --upgrade pip setuptools wheel pyinstaller requests

:: -----------------------------------------------
:: Create launcher batch file
:: -----------------------------------------------
(
echo @echo off
echo SETLOCAL
echo set "BASE_DIR=%%~dp0"
echo if exist "%%BASE_DIR%%python\python.exe" (set "PYTHON_BIN=%%BASE_DIR%%python\python.exe") else (set "PYTHON_BIN=python")
echo set "PACKAGE_PY=%%BASE_DIR%%package.py"
echo if not exist "!PACKAGE_PY!" (
echo     echo ❌ package.py not found!
echo     exit /b 1
echo )
echo "!PYTHON_BIN!" "!PACKAGE_PY!" %%*
) > "%INSTALL_DIR%\package.bat"


echo ✅ Installation complete!
echo Run your package with: "%INSTALL_DIR%\package.bat" -install nevo
