@echo off
setlocal
set from=%1

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat

Title Building filehash for 64 bit Windows

erase *.obj *.exe
make -j 4 SHELL="%ComSpec%" -f ..\Makefile intel_win
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
if x%from% == xbot goto skip
pause
:skip
