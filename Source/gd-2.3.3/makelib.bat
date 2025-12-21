@echo off
setlocal
call ..\scripts\setopts %*
title Building windows gd library
erase *.o *.obj libgd.a libgd.lib
set target=libgd.lib
if %COMPILER% == gcc set target=libgd.a
if exist finished erase finished
make CFLAGS="-DBGDWIN32 -DWIN32 -DENABLE_FREETYPE=OFF  -DHAVE_LIBPNG -DHAVE_LIBZ -DHAVE_LIBJPEG " COMPILER=%COMPILER% LIB=%LIB% SIZE=%SIZE% RM=erase -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
echo finished > finished
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
