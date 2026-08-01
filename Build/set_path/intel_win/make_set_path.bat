@echo off

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat

Title Building Windows setpath
erase *.obj *.exe
make SHELL="%ComSpec%" -f ../Makefile intel_win
