@echo off

call ..\..\scripts\set_smv_opts %*

:: setup compiler environment
if x%from% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

set SMV_TESTFLAG=
set SMV_TESTSTRING=

Title Building Smokeview for 64 bit Windows
set SMV_TESTFLAG=
set SMV_TESTSTRING=
if "%release%" == "-r" goto endif
  Title Building Test Smokeview for 64 bit Windows
  set SMV_TESTFLAG=-D pp_BETA
  set SMV_TESTSTRING=test_
:endif

IF NOT DEFINED ONEAPI_ROOT goto skip_oneapi
  set SMV_TESTFLAG=%SMV_TESTFLAG% -D pp_WIN_ONEAPI
:skip_oneapi

if NOT x%GLUT% == xfreeglut set GLUT=glut

if x%inc% == xinc goto skip_inc
erase *.obj *.mod *.exe 2> Nul
:skip_inc

make -j 4 ICON="%ICON%" GLUT="%GLUT%" SHELL="%ComSpec%" SMV_TESTFLAG="%SMV_TESTFLAG%" SMV_TESTSTRING="%SMV_TESTSTRING%" -f ..\Makefile intel_win_64%debug%
if x%from% == xbot goto skip2
pause
:skip2

