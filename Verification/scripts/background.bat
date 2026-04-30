@echo off
setlocal ENABLEDELAYEDEXPANSION

set "MAX_INSTANCES=5"
set "PROCESS_NAME=fds_impi_intel_win.exe"

if "%~1"=="" (
    echo Error: No command specified
    exit /b 1
)

REM Build command string
set "CMD=%*"

echo Waiting until %MAX_INSTANCES% or fewer instances of %PROCESS_NAME% are running...
echo Command to run: %CMD%
echo.

:CHECK
set COUNT=0

REM Count running instances
for /f "skip=3 tokens=1" %%A in ('tasklist /FI "IMAGENAME eq %PROCESS_NAME%"') do (
    if /I "%%A"=="%PROCESS_NAME%" (
        set /a COUNT+=1
    )
)

echo Current %PROCESS_NAME% instances: !COUNT!

if !COUNT! LEQ %MAX_INSTANCES% (
    echo.
    echo Instance count is within limit. Running command...
    echo.
    call %CMD%
    exit /b %ERRORLEVEL%
)

REM Wait 5 seconds and try again
timeout /t 5 /nobreak >nul
goto :CHECK