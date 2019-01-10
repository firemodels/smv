@echo off
set from=%1

Title Building make_time for 64 bit Windows

erase *.o *.exe
make -f ..\Makefile gnu_win_64

if x%from% == xbot goto skip_pause
pause
:skip_pause
