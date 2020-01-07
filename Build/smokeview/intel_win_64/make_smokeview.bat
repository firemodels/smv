@echo off

call ..\scripts\set_smv_opts %*

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
set OPT=
if  "x%VS140COMNTOOLS%" == "x" goto endif2
  set OPT=-DHAVE_SNPRINTF -DHAVE_STRUCT_TIMESPEC
:endif2
if "%smv_mpi%" == "false" goto skip_mpi
  set SMV_TESTFLAG=%SMV_TESTFLAG% -D pp_MPI
:skip_mpi


if NOT x%GLUT% == xfreeglut set GLUT=glut

if x%inc% == xinc goto skip_inc
erase *.obj *.mod *.exe
:skip_inc

make -j 4 ICON="%ICON%" GLUT="%GLUT%" INTEL_MPI="%smv_mpi%" I_MPI_ROOT="%I_MPI_ROOT%" SHELL="%ComSpec%" SMV_TESTFLAG="%SMV_TESTFLAG% %OPT%" SMV_TESTSTRING="%SMV_TESTSTRING%" -f ..\Makefile intel_win_64
if x%from% == xbot goto skip2
pause
:skip2

