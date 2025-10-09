@echo off
setlocal
call ..\scripts\setopts %*
title Building openvr library
erase *.o *.obj libopenvr.a libopenvr.lib
set target=libopenvr.lib

make COMPILER=%COMPILER% LIB=%LIB% SIZE=%SIZE% RM=erase -f ./Makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
endlocal
