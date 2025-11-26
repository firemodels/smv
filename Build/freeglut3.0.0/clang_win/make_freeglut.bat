@echo off
setlocal
title Building freeglut library

call ..\..\..\Source\scripts\setopts %*
title Building freeglut library
erase *.obj *.lib

make SHELL="%ComSpec%" PLATFORM="win" -f ..\Makefile clang_win
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
endlocal

