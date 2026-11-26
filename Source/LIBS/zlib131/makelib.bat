@echo off
setlocal
call ..\..\scripts\setopts %*
title Building zlib library
git clean -dxf
set target=libz.lib
if %COMPILER% == gcc set target=libz.a
if exist finished erase finished
make COMPILER=%COMPILER% LIB=%SMV_LIB_TOOL% SIZE=%SIZE% -f ./makefile %target%
echo finished > finished
endlocal
