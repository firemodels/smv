@echo off
set arg1=%1

Title Building convert for 64 bit Windows

erase *.obj *.mod
make SHELL="%ComSpec%" -f ..\Makefile clang_win_64
if x%arg1% == xbot goto skip2
pause
:skip2

