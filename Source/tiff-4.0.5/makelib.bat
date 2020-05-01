@echo off
setlocal
call ..\scripts\setopts %*
title Building windows tifflibrary
erase *.o *.obj libtiff.a libtiff.lib
set target=libtiff.lib
make COMPILER=%COMPILER% SHELL="%ComSpec%" ./makefile %target%
endlocal
