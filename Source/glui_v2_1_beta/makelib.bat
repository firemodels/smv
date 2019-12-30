@echo off
setlocal
call ..\scripts\setopts %*
title Building windows glui library
erase *.o *.obj libglui.a libglui.lib

set target=intel_win_64
if %COMPILER% == gcc set target=gnu_win_64

set GLUTTYPE=glut
if x%glutopt% EQU xfreeglut set GLUTTYPE=freeglut

make GLUT="%GLUTTYPE%" COMPILER=%COMPILER% COMPILER2=%COMPILER2% -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%

if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
