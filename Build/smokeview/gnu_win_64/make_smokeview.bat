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

erase *.obj *.mod
make SHELL="%ComSpec%" -f ..\Makefile gnu_win_64
pause

