@echo off
setlocal
title Building freeglut library

call ..\..\..\Source\scripts\setopts %*
title Building freeglut library
erase *.o *.obj *.lib *.a

make SHELL="%ComSpec%" PLATFORM="win" -f ..\Makefile intel_win_64
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
endlocal

