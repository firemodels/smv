@echo off
set from=%1

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat

Title Building mergepo for 64 bit Windows

erase *.obj *.mod *.exe
make -j 4 SHELL="%ComSpec%" -f ..\Makefile intel_win_64
if x%from% == xbot goto skip2
pause
:skip2
