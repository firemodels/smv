@echo off
set arg1=%1

Title Building env2mod for 64 bit Windows

erase *.o *.mod
make SHELL="%ComSpec%" -f ..\Makefile gnu_win_64
if x%arg1% == xbot goto skip2
pause
:skip2

