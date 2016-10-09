@echo off
setlocal
call ..\scripts\setopts %*
title Building glut library
erase *.o *.obj libglutwin.a libglutwin.lib
set target=libglutwin.lib
if %COMPILER% == gcc set target=libglutwin.a
make COMPILER=%COMPILER% SIZE=%SIZE% FILTERC="-D WIN32 -D _WIN32" -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
endlocal
