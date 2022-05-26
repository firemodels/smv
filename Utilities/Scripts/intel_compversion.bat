@echo off
set arg1=%1
call :is_file_installed head|| exit /b 1
call :is_file_installed sed || exit /b 1
call :is_file_installed gawk|| exit /b 1
call :is_file_installed icl || exit /b 1

set ICL=icx
if x%arg1% == x goto endif1
if %arg1% == icl goto else1
set ICL=icx
goto endif1
:else1
set ICL=%arg1%
:endif1

set CTYPE=DPC++/C++
if "x%INTEL_ICC%" == "x" goto skip_setintel
  set ICL=%INTEL_ICC%
:skip_setintel

if NOT %ICL% == icl goto skipclassic
set CTYPE=C/C++ Classic
:skipclassic


%ICL% > c_version.txt 2>&1
head -1 c_version.txt | sed "s/^.*\(Version.*\).*$/\1/" | gawk "{print $2}" > vers.out
set /p vers=<vers.out
echo "Intel %CTYPE% %vers%"
erase c_version.txt vers.out
goto eof

:: -------------------------------------------------------------
:is_file_installed
:: -------------------------------------------------------------

  set program=%1
  %program% --help 1> output.txt 2>&1
  type output.txt | find /i /c "not recognized" > output_count.txt
  set /p nothave=<output_count.txt
  if %nothave% == 1 (
    echo unknown
    erase output.txt output_count.txt
    exit /b 1
  )
  erase output.txt output_count.txt
  exit /b 0

:eof


