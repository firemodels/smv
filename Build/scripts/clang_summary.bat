@echo off
setlocal enabledelayedexpansion
set OUTFILE=%1
set NWARNINGFILE=%TEMP%\nwarnings.txt
if not "x%OUTFILE%" == "x" goto endif1
  set OUTFILE=clang_warnings.txt
:endif1

set HAVEWARNINGS=0

for %%f in (*.winchk) do (
  call :GetFileSize %%f filesize
  if not "%filesize%" == "0" set HAVEWARNING=1
  if not "%filesize%" == "0" goto loop1
)
:loop1


echo clang warnings > %OUTFILE%
echo -------------- >> %OUTFILE%
if not "x%HAVEWARNINGS%" == "x" goto endif2
  echo no warnings were found > %OUTFILE%
  exit /b
fi
:endif2

set total_warnings=0
set nfiles=0
set nwarnfiles=0

for %%f in ( *.winchk ) do (
  set /a nfiles=!nfiles!+1
  call :GetFileSize %%f filesize
  if not "!filesize!" == "0" tail -1 %%f | gawk "{print $1}" > %NWARNINGFILE%
  if not "!filesize!" == "0" set /p nwarnings=<%NWARNINGFILE%
  if not "!filesize!" == "0" set /a total_warnings=!total_warnings!+!nwarnings!"
  if not "!filesize!" == "0" set /a nwarnfiles=!nwarnfiles!+1
  if not "!filesize!" == "0" echo %%f: !nwarnings!  warnings >> %OUTFILE%
  if not "!filesize!" == "0" echo %%f: !nwarnings!  warnings
)
echo.
if not "%total_warnings%" == "0" goto else5
  echo %nfiles% files scanned, no warnings were found
  echo %nfiles% files scanned, no warnings were found >> %OUTFILE%
  goto endif5
:else5
  echo %nfiles% files scanned, %nwarnfiles% files have %total_warnings% warnings
  echo %nfiles% files scanned, %nwarnfiles% files have %total_warnings% warnings >> %OUTFILE%
:endif5

goto eof

::--------------------------------------------------
:GetFileSize
::--------------------------------------------------
set filesize=%~z1
set %2=%filesize%
exit /b

:eof