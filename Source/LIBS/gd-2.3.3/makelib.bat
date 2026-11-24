@echo off
setlocal
call ..\scripts\setopts %*
title Building gd library
git clean -dxf
set target=libgd.lib
if %COMPILER% == gcc set target=libgd.a
if exist finished erase finished
make CFLAGS="-DBGDWIN32 -DWIN32 -DENABLE_FREETYPE=OFF  -DHAVE_LIBPNG -DHAVE_LIBZ -DHAVE_LIBJPEG " COMPILER=%COMPILER% LIB=%LIB% SIZE=%SIZE% RM=erase -f ./makefile %target%
echo finished > finished
endlocal
