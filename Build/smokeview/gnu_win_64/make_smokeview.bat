@echo off
set release=%1
set from=%2
set GLUT=%3

:: call ..\..\scripts\test_libs ..\..\LIBS

set SMV_TESTFLAG=
set SMV_TESTSTRING=

Title Building Smokeview for 64 bit Windows
set SMV_TESTFLAG=
set SMV_TESTSTRING=
set OPT=

if NOT x%GLUT% == xfreeglut set GLUT=glut

erase *.o *.mod *.exe
make GLUT="%GLUT%" SHELL="%ComSpec%" gnu_win_64 -f ..\Makefile
pause

