@echo off
setlocal
call ..\scripts\setopts %*
title Building windows jpeg library
erase *.o *.obj libjpeg.a libjpeg.lib
set target=libjpeg.lib
if %COMPILER% == gcc set target=libjpeg.a
if exist finished erase finished
make COMPILER=%COMPILER% LIB=%LIB% SIZE=%SIZE% RM=erase -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
echo finished > finished
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
