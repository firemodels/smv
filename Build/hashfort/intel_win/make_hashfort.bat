@echo off
set release=%1
set from=%2

:: setup compiler environment
if x%from% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

Title Building hashfort for 64 bit Windows

erase *.obj *.exe *.mod
make SHELL="%ComSpec%"  -f ../Makefile intel_win
if x%from% == xbot goto skip2
pause
:skip2
