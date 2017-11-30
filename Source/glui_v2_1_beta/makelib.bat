@echo off
setlocal
call ..\scripts\setopts %*
title Building glui library
erase *.o *.obj libglui.a libglui.lib
set target=libglui.lib
if %COMPILER% == gcc set target=libglui.a
make COMPILER=%COMPILER% COMPILER2=%COMPILER2% SIZE=%SIZE% RM=erase intel_win_64 -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
endlocal
