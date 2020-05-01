@echo off
setlocal
call ..\scripts\setopts %*
title Building windows tifflibrary
erase *.o *.obj libtiff.a libtiff.lib
set target=libtiff.lib
make -j 4 COMPILER=%COMPILER% SHELL="%ComSpec%" ./makefile %target%
endlocal
