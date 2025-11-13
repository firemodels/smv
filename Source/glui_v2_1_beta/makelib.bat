@echo off
setlocal
call ..\scripts\setopts %*
title Building windows glui library
erase *.o *.obj libglui.a libglui.lib

set target=intel_win
if %COMPILER% == gcc set target=gnu_win
if %COMPILER% == clang-cl set target=clang_win

set GLUTTYPE=glut
if x%glutopt% EQU xfreeglut set GLUTTYPE=freeglut

if exist finished erase finished
make GLUT="%GLUTTYPE%" COMPILER=%COMPILER% COMPILER2=%COMPILER2% LIB=%LIB% -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
echo finished > finished

if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
