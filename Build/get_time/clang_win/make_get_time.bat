@echo off
set from=%1

Title Building get_time for Windows

erase *.obj *.exe
make -f ..\Makefile clang_win

if x%from% == xbot goto skip_pause
pause
:skip_pause
