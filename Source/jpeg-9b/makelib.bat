@echo off
setlocal
call ..\scripts\setopts %*
title Building windows jpeg library
erase *.o *.obj libjpeg.a libjpeg.lib
set target=libjpeg.lib
if %COMPILER% == gcc set target=libjpeg.a
make COMPILER=%COMPILER% SIZE=%SIZE% RM=erase -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
