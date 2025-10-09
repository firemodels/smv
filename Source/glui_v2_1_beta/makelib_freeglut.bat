@echo off
setlocal
call ..\scripts\setopts %*
title Building glui library
erase *.o *.obj libglui.a libglui.lib

set target=intel_win_64
if %COMPILER% == gcc set target=gnu_win_64

make GLUT="freeglut" COMPILER=%COMPILER% COMPILER2=%COMPILER2% LIB=%LIB% -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
endlocal
