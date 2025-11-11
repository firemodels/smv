@echo off
:: setup compiler environment
call ..\..\..\UtilitiesScripts\setup_intel_compilers.bat

Title Building debug smokediff for 64 bit Windows

erase *.obj *.mod *.exe
make -f ..\Makefile intel_win_db
pause

