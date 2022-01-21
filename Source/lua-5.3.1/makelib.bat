@echo off
setlocal
call ..\scripts\setopts %*
title Building windows lua library
erase *.o *.obj liblua.a liblua.lib
set target=windows
make CC=%COMPILER% SIZE=%SIZE% AR=lib AR_OUT="/OUT:" RANLIB=dir RM=erase -f ./Makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
