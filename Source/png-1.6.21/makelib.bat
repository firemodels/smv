@echo off
setlocal
call ..\scripts\setopts %*
title Building windows png library
erase *.o *.obj libpng.a libpng.lib
set target=libpng.lib
if %COMPILER% == gcc set target=libpng.a
make COMPILER=%COMPILER% SIZE=%SIZE% RM=erase -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
