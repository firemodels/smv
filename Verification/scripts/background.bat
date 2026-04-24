@echo off
setlocal ENABLEDELAYEDEXPANSION

REM ==== CONFIG ====
REM %1 = max CPU load threshold (e.g. 30)
REM %2... = command + args to run

if "%~1"=="" (
    echo Usage: %~nx0 max_cpu_percent command [args...]
    exit /b 1
)

set "THRESHOLD=%~1"
shift

if "%~1"=="" (
    echo Error: No command specified
    exit /b 1
)

REM Build command string
set "CMD=%*"

echo Waiting for CPU load to drop below %THRESHOLD%%%...
echo Command to run: %CMD%
echo.

:CHECK
REM Query CPU load
for /f "skip=1 tokens=*" %%A in ('wmic cpu get loadpercentage') do (
    if not "%%A"=="" (
        set "LOAD=%%A"
        goto :GOTLOAD
    )
)

:GOTLOAD
REM Trim spaces
set "LOAD=%LOAD: =%"

if "%LOAD%"=="" goto :CHECK

echo Current CPU Load: %LOAD%%%

if %LOAD% LSS %THRESHOLD% (
    echo.
    echo CPU load is below threshold. Running command...
    echo.
    call %CMD%
    exit /b %ERRORLEVEL%
)

REM Wait 5 seconds and try again
timeout /t 5 /nobreak >nul
goto :CHECK