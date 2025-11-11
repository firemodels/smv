@echo off
set arg1=%1

:: setup compiler environment
if x%arg1% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

Title Building smokediff for 64 bit Windows

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\

set SMV_TESTFLAG=
if x%ONEAPI_FORT_CAPS% == x1 set SMV_TESTFLAG=%SMV_TESTFLAG% -D pp_WIN_ONEAPI

erase *.obj *.mod *.exe
make -j 4 SHELL="%ComSpec%" SMV_TESTFLAG="%SMV_TESTFLAG%"  -f ..\Makefile intel_win
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
if x%arg1% == xbot goto skip2
pause
:skip2
