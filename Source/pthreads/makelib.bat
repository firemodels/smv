@echo off
setlocal
call ..\scripts\setopts %*
title Building windows pthread library
erase *.o *.obj libpthread.a libpthreads.lib
set target=libpthreads.lib
set CFLAGS=
if %COMPILER% == gcc set target=libpthreads.a
if %COMPILER% == gcc set CFLAGS=

set OPT=
if  NOT "x%COMPILER%" == "xicl" goto endif2
  set OPT=-DHAVE_STRUCT_TIMESPEC
:endif2
if  NOT "x%COMPILER%" == "xicx" goto endif3
  set OPT=-DHAVE_STRUCT_TIMESPEC
:endif3
set OPT=-DHAVE_STRUCT_TIMESPEC

make CFLAGS=%CFLAGS% COMPILER=%COMPILER% SIZE=%SIZE% OPT=%OPT% RM=erase -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
