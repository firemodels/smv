@echo off
set arg1=%1

:: setup compiler environment
if x%arg1% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

:: call ..\..\scripts\test_libs ..\..\LIBS

set OPT=
if  "x%HAVE_MSVS%" == "x" goto endif2
  set OPT=-DHAVE_MSVS
:endif2

Title Building smokezip for 64 bit Windows

erase *.obj *.mod *.exe
make SHELL="%ComSpec%" OPT="%OPT%" -f ..\Makefile intel_win_64
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
if x%arg1% == xbot goto skip2
pause
:skip2

