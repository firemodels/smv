@echo off
set release=%1
set from=%2

:: call ..\..\scripts\test_libs ..\..\LIBS

set SMV_TESTFLAG=
set SMV_TESTSTRING=

Title Building Smokeview for 64 bit Windows
set SMV_TESTFLAG=
set SMV_TESTSTRING=
set OPT=

erase *.o *.mod
make SHELL="%ComSpec%" gnu_win_64 -f ..\Makefile
pause

