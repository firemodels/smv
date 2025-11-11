@echo off
set arg1=%1

:: setup compiler environment
if x%arg1% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

Title Building wind2fds for 64 bit Windows

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\

erase *.obj *.mod *.exe
make -j 4 SHELL="%ComSpec%" -f ..\Makefile intel_win
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
if x%arg1% == xbot goto skip2
pause
:skip2
