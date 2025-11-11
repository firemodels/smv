@echo off
set from=%1

Title Building set_path for 64 bit Windows
erase *.obj *.exe
make SHELL="%ComSpec%" -f ../Makefile clang_win
if x%from% == xbot goto skip2
pause
:skip2
