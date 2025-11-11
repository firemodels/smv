@echo off

call ..\..\scripts\set_smv_opts %*

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat

set SMV_TESTFLAG=
set SMV_TESTSTRING=

Title Building Smokeview for 64 bit Windows
if "%1" NEQ "-t" goto endif
  Title Building Test Smokeview for 64 bit Windows
  set SMV_TESTFLAG=-D pp_BETA
  set SMV_TESTSTRING=test_
:endif

if x%ONEAPI_FORT_CAPS% == x1 set SMV_TESTFLAG=%SMV_TESTFLAG% -D pp_WIN_ONEAPI

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\

make -j %NUMBER_OF_PROCESSORS% ICON="%ICON%" GLUT="%GLUT%" SHELL="%ComSpec%" SMV_TESTFLAG="%SMV_TESTFLAG%" SMV_TESTSTRING="%SMV_TESTSTRING%" -f ..\Makefile intel_win%debug%
pause
