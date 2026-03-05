@echo off
setlocal
call ..\scripts\setopts %*
title Building jpeg library
git clean -dxf
set target=libjpeg.lib
if %COMPILER% == gcc set target=libjpeg.a
if exist finished erase finished
make COMPILER=%COMPILER% LIB=%LIB% SIZE=%SIZE% RM=erase -f ./makefile %target%
echo finished > finished
endlocal
