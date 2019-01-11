@echo off
setlocal
call ..\scripts\setopts %*
title Building pthread library
erase *.o *.obj libpthread.a libpthreads.lib
set target=libpthreads.lib
set CFLAGS=-o2
if %COMPILER% == gcc set target=libpthreads.a
if %COMPILER% == gcc set CFLAGS=

set OPT=
if  "x%VS140COMNTOOLS%" == "x" goto endif2
  set OPT=-DHAVE_STRUCT_TIMESPEC
:endif2

make CFLAGS=%CFLAGS% COMPILER=%COMPILER% SIZE=%SIZE% OPT=%OPT% RM=erase -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
endlocal
