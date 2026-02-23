@echo off
setlocal
call ..\scripts\setopts %*
title Building windows jpeg library
erase *.o *.obj libjpeg.a libjpeg.lib
set target=libjpeg.lib
if %COMPILER% == gcc set target=libjpeg.a
if exist finished erase finished
make COMPILER=%COMPILER% LIB=%LIB% SIZE=%SIZE% RM=erase -f ./makefile %target%
echo finished > finished
endlocal
