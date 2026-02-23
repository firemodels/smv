@echo off
setlocal
call ..\scripts\setopts %*
title Building png library
git clean -dxf
set target=libpng.lib
if %COMPILER% == gcc set target=libpng.a
if exist finished erase finished
make COMPILER=%COMPILER% LIB=%LIB% SIZE=%SIZE% RM=erase -f ./makefile %target%
echo finished > finished
endlocal
