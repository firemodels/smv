@echo off

Title Building getdate for Windows

erase *.obj *.exe
make SHELL="%ComSpec%" -f ..\Makefile clang_win
if x%arg1% == xbot goto skip2
pause
:skip2


