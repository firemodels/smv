@echo off
setlocal
call ..\..\scripts\setopts %*
title Building glui library
git clean -dxf

set target=intel_win
if %COMPILER% == gcc set target=gnu_win

make GLUT="freeglut" COMPILER=%COMPILER% COMPILER2=%COMPILER2% LIB=%SMV_LIB_TOOL% -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
endlocal
