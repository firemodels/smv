@echo off
setlocal
set from=%1

Title Building filehash for 64 bit Windows

erase *.obj
make SHELL="%ComSpec%" -f ..\Makefile clang_win_64

if x%from% == xbot goto skip
pause
:skip
