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

if NOT x%GLUT% == xfreeglut set GLUT=glut

if x%ONEAPI_FORT_CAPS% == x1 set SMV_TESTFLAG=%SMV_TESTFLAG% -D pp_WIN_ONEAPI

if x%inc% == xinc goto skip_inc
erase *.obj *.mod *.exe 2> Nul
:skip_inc

copy ..\..\..\Source\smvluacore\*.lua .
copy ..\..\..\Build\LIBS\intel_win_64\*.dll
make -j 4 ICON="%ICON%" GLUT="%GLUT%" SHELL="%ComSpec%" LUA_SCRIPTING="true" SMV_TESTFLAG="%SMV_TESTFLAG%" SMV_TESTSTRING="%SMV_TESTSTRING%" -f ..\Makefile intel_win_64%debug%
if x%from% == xbot goto skip2
pause
:skip2
