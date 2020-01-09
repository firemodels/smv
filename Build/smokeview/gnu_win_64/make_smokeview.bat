@echo off

call ..\..\scripts\set_smv_opts %*  || exit /b 1

if NOT x%setglut% == x goto skip_setglut
  set GLUT=glut
:skip_setglut

call ..\..\scripts\test_gnu_libs ..\..\LIBS %GLUT%

Title Building Smokeview for 64 bit Windows
set SMV_TESTFLAG=
set SMV_TESTSTRING=
set OPT=

if "%release%" == "-r" goto endif
  Title Building Test Smokeview for 64 bit Windows
  set SMV_TESTFLAG=-D pp_BETA
  set SMV_TESTSTRING=test_
:endif

if NOT x%GLUT% == xglut set GLUT=freeglut

erase *.o *.mod *.exe
make -j 4 GLUT="%GLUT%" SHELL="%ComSpec%" SMV_TESTFLAG="%SMV_TESTFLAG% %OPT%" SMV_TESTSTRING="%SMV_TESTSTRING%" SMV_PROFILEFLAG="%SMV_PROFILEFLAG%" SMV_PROFILESTRING="%SMV_PROFILESTRING%" -f ..\Makefile gnu_win_64%debug%

if x%from% == xbot goto skip2
pause
:skip2

