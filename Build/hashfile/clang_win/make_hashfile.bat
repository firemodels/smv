@echo off
setlocal
set from=%1

Title Building hashfile for 64 bit Windows

erase *.obj *.exe
make SHELL="%ComSpec%" -f ..\Makefile clang_win

if x%from% == xbot goto skip
pause
:skip
