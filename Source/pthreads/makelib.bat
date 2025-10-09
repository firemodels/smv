@echo off
setlocal
call ..\scripts\setopts %*
title Building windows pthread library
erase *.o *.obj libpthread.a libpthreads.lib
set target=libpthreads.lib
set CFLAGS=
if %COMPILER% == gcc set target=libpthreads.a
if %COMPILER% == gcc set CFLAGS=
if %COMPILER% == clang-cl set CFLAGS=

set OPT=
if  NOT "x%COMPILER%" == "xicl" goto endif2
  set OPT=-DHAVE_STRUCT_TIMESPEC
:endif2
if  NOT "x%COMPILER%" == "xicx" goto endif3
  set OPT=-DHAVE_STRUCT_TIMESPEC
:endif3
set OPT=-DHAVE_STRUCT_TIMESPEC

if exist finished erase finished
make CFLAGS=%CFLAGS% COMPILER=%COMPILER% LIB=%LIB% SIZE=%SIZE% OPT=%OPT% RM=erase -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
echo finished > finished
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
