@echo off
set from=%1

Title Building mergepo for Windows

:: build libraries if one is missing
call ..\..\scripts\test_clang_libs.bat ..\..\LIBS\

erase *.obj *.exe
make -j 4 SHELL="%ComSpec%" -f ..\Makefile clang_win
if x%from% == xbot goto skip2
pause
:skip2
