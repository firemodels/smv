@echo off
set release=%1
set from=%2

:: setup compiler environment
if x%from% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

title Building freeglut library
erase *.obj *.lib
make SHELL="%ComSpec%" PLATFORM="win" -f ..\Makefile intel_win_64
if x%from% == xbot goto skip2
pause
:skip2

