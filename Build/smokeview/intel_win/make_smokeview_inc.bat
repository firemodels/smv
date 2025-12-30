@echo off

call ..\..\scripts\set_smv_opts %*

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat

set SMV_TESTFLAG=
Title Building Smokeview for Windows
if x%ONEAPI_FORT_CAPS% == x1 set SMV_TESTFLAG=%SMV_TESTFLAG% -D pp_WIN_ONEAPI

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\

make -j %NUMBER_OF_PROCESSORS% ICON="%ICON%" GLUT="%GLUT%" SHELL="%ComSpec%" SMV_TESTFLAG="%SMV_TESTFLAG%" -f ..\Makefile intel_win%debug%
pause
