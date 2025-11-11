@echo off
:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat

Title Building debug smokediff for 64 bit Windows

set SMV_TESTFLAG=
if x%ONEAPI_FORT_CAPS% == x1 set SMV_TESTFLAG=%SMV_TESTFLAG% -D pp_WIN_ONEAPI

erase *.obj *.mod *.exe
make SHELL="%ComSpec%" SMV_TESTFLAG="%SMV_TESTFLAG%" -f ..\Makefile intel_win_db
pause
