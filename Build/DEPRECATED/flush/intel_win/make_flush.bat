@echo off

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat

Title Building flushcache for Windows

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\

erase *.obj *.exe
make -j 4 SHELL="%ComSpec%" -f ..\Makefile intel_win
