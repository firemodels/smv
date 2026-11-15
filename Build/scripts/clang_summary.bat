@echo off
setlocal enabledelayedexpansion
set OUTFILE=%1
if not "x%OUTFILE%" == "x" goto endif1
  set OUTFILE=clang_warnings.txt
:endif1

echo clang warnings > "%OUTFILE%"
echo -------------- >> "%OUTFILE%"

set total_warnings=0
set nfiles=0
set nwarnfiles=0

for %%f in ( *.winchk ) do (
  set /a nfiles=!nfiles!+1
  set nwarnings=0
  for /f "tokens=1" %%n in ('findstr /r /x /c:"[0-9][0-9]* warning generated\." /c:"[0-9][0-9]* warnings generated\." "%%f"') do set nwarnings=%%n
  if not "!nwarnings!" == "0" set /a total_warnings+=nwarnings
  if not "!nwarnings!" == "0" set /a nwarnfiles=!nwarnfiles!+1
  if not "!nwarnings!" == "0" (
    if !nwarnfiles! gtr 1 echo -------------------->> "%OUTFILE%"
    echo %%~nf>> "%OUTFILE%"
    type "%%f">> "%OUTFILE%"
    echo.>> "%OUTFILE%"
  )
  if not "!nwarnings!" == "0" echo %%f: !nwarnings!  warnings
)
echo.
if not "%total_warnings%" == "0" goto else5
  echo %nfiles% files scanned, no warnings were found
  echo %nfiles% files scanned, no warnings were found >> "%OUTFILE%"
  goto endif5
:else5
  echo %nfiles% files scanned, %nwarnfiles% files have %total_warnings% warnings
  echo %nfiles% files scanned, %nwarnfiles% files have %total_warnings% warnings >> "%OUTFILE%"
:endif5

exit /b 0
