@echo off
call :is_file_installed head|| exit /b 1
call :is_file_installed sed || exit /b 1
call :is_file_installed gawk|| exit /b 1
call :is_file_installed gcc || exit /b 1

gcc --version > c_version.txt 2>&1
head -1 c_version.txt |  gawk "{print $NF}" > vers.out
set /p vers=<vers.out
echo "gcc C/C++ %vers%"
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


