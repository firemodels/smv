@echo off
set from=%1

Title Building set_path for 64 bit Windows
erase *.obj *.exe
make SHELL="%ComSpec%" -f ../Makefile clang_win_64
if x%from% == xbot goto skip2
pause
:skip2
