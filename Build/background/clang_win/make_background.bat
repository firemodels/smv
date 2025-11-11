@echo off
set arg1=%1

Title Building background for 64 bit Windows

:: build libraries if one is missing
call ..\..\scripts\test_clang_libs.bat ..\..\LIBS\

erase *.obj *.exe
make SHELL="%ComSpec%" -f ..\Makefile clang_win
if x%arg1% == xbot goto skip2
pause
:skip2
