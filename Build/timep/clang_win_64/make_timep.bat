@echo off
set arg1=%1

Title Building timep for 64 bit Windows

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\

erase *.obj *.exe
make SHELL="%ComSpec%" -f ..\Makefile clang_win_64
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
if x%arg1% == xbot goto skip2
pause
:skip2
