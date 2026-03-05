@echo off

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_compilers.bat intel

Title Building pnginfo for Windows

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\intel_win

erase *.obj *.exe
make SHELL="%ComSpec%" -f ..\Makefile intel_win