@echo off
set from=%1

Title Building get_time for 64 bit Windows

erase *.obj *.exe
make -f ..\Makefile clang_win_64

if x%from% == xbot goto skip_pause
pause
:skip_pause
