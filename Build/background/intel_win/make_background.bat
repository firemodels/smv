@echo off

:: setup compiler environment
if not defined ONEAPI_ROOT call ..\..\..\Utilities\Scripts\setup_compilers.bat intel

Title Building background for Windows

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\intel_win

erase *.obj *.mod *.exe
make -j 4 SHELL="%ComSpec%" -f ..\Makefile intel_win