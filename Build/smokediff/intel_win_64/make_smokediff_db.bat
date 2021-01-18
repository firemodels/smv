@echo off
:: setup compiler environment
call ..\..\..\UtilitiesScripts\setup_intel_compilers.bat

Title Building debug smokediff for 64 bit Windows

set SMV_TESTFLAG=
IF NOT DEFINED ONEAPI_ROOT goto skip_oneapi
  set SMV_TESTFLAG=%SMV_TESTFLAG% -D pp_WIN_ONEAPI
:skip_oneapi

erase *.obj *.mod *.exe
make SMV_TESTFLAG="%SMV_TESTFLAG%" -f ..\Makefile intel_win_64_db
pause

