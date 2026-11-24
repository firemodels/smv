@echo off
setlocal enabledelayedexpansion
set OUTFILE=%1
if not "x%OUTFILE%" == "x" goto endif1
  set OUTFILE=clang_warnings.txt
:endif1

echo clang warnings > "%OUTFILE%"
echo -------------- >> "%OUTFILE%"

set "dashes=--------------------------------------------------------------------------------"
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
    set "filename=%%~nf"
    call :file_header
    type "%%f">> "%OUTFILE%"
    echo.>> "%OUTFILE%"
  )
  if not "!nwarnings!" == "0" echo %%f: !nwarnings!  warnings
)
echo.
echo !dashes!>> "%OUTFILE%"
if not "%total_warnings%" == "0" goto else5
  echo %nfiles% files scanned, no warnings were found
  echo %nfiles% files scanned, no warnings were found >> "%OUTFILE%"
  goto endif5
:else5
  echo %nfiles% files scanned, %nwarnfiles% files have %total_warnings% warnings
  echo %nfiles% files scanned, %nwarnfiles% files have %total_warnings% warnings >> "%OUTFILE%"
:endif5

exit /b 0

:file_header
set "remaining=!filename!"
set ndashes=80
:count_filename
if defined remaining if !ndashes! gtr 0 (
  set "remaining=!remaining:~1!"
  set /a ndashes-=1
  goto count_filename
)
set /a left=ndashes/2, right=ndashes-left
for /f "tokens=1,2" %%a in ("!left! !right!") do echo(!dashes:~0,%%a! !filename! !dashes:~0,%%b!>> "%OUTFILE%"
exit /b 0
