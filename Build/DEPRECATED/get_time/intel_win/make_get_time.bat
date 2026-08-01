@echo off
:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat

Title Building make_time for Windows

erase *.obj *.mod *.exe
make -f ..\Makefile intel_win
