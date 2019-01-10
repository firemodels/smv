@echo off
set arg1=%1

Title Building dem2fds for 64 bit Windows

erase *.o
make SHELL="%ComSpec%" -f ..\Makefile gnu_win_64
if x%arg1% == xbot goto skip2
pause
:skip2

