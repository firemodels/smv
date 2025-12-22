@echo off
set arg1=%1

Title Building gdtest for Windows

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\

erase *.obj *.exe
make SHELL="%ComSpec%" -f ..\Makefile clang_win
