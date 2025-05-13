@echo off
setlocal
call ..\scripts\setopts %*
title Building windows png library
erase *.o *.obj libpng.a libpng.lib
set target=libpng.lib
if %COMPILER% == gcc set target=libpng.a
if exist finished erase finished
make COMPILER=%COMPILER% SIZE=%SIZE% RM=erase -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
echo finished > finished
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
