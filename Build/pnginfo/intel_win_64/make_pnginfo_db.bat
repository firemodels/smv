@echo off
set arg1=%1

:: setup compiler environment
if x%arg1% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

Title Building pnginfo for 64 bit Windows

erase *.obj *.mod *.exe
make SHELL="%ComSpec%" -f ..\Makefile intel_win_64_db
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
if x%arg1% == xbot goto skip2
pause
:skip2

