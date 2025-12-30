@echo off

call ..\..\scripts\set_smv_opts %*

:: setup compiler environment
if x%from% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

Title Building Smokeview for Windows
set SMV_TESTFLAG=

if NOT x%GLUT% == xfreeglut set GLUT=glut

if x%ONEAPI_FORT_CAPS% == x1 set SMV_TESTFLAG=%SMV_TESTFLAG% -D pp_WIN_ONEAPI

if x%inc% == xinc goto skip_inc
erase *.obj *.mod *.exe
:skip_inc

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\

make -j %NUMBER_OF_PROCESSORS% ICON="%ICON%" GLUT="%GLUT%" SHELL="%ComSpec%" SMV_TESTFLAG="%SMV_TESTFLAG%" -f ..\Makefile intel_win_san

if x%from% == xbot goto skip2
pause
:skip2
goto eof

:: -------------------------------------------------------------
  :find_smokeview_warnings
:: -------------------------------------------------------------

set search_file=%1

grep -v "commands for target" %search_file% > build_warning0.txt
grep -i -A 1 -B 1 remark build_warning0.txt > build_warnings.txt
grep -i -A 1 -B 1 warning build_warning0.txt >> build_warnings.txt
type build_warnings.txt | find /v /c "kdkwokwdokwd"> build_nwarnings.txt
set /p nwarnings=<build_nwarnings.txt
if %nwarnings% GTR 0 (
  echo Warnings:
  echo.
  type build_warnings.txt
)
exit /b

:eof
