@echo off
setlocal
call ..\scripts\setopts %*
title Building glut library
erase *.o *.obj libglutwin.a libglutwin.lib

set target=libglutwin.lib
if %COMPILER% == gcc set target=libglutwin.a

set mingwparms=
if %COMPILER% == gcc set minggwparms=-D __MINGW32__ -Wno-pointer-to-int-cast

make COMPILER=%COMPILER% SIZE=%SIZE% FILTERC="-D WIN32 -D _WIN32 %minggwparms% " -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
endlocal
