@echo off
setlocal
call ..\scripts\setopts %*
title Building windows tifflibrary
erase *.o *.obj libtiff.a libtiff.lib
set target=libtiff.lib
make -j 4 COMPILER=%COMPILER% SHELL="%ComSpec%" ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
