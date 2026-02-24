@echo off

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_compilers.bat intel

Title Building fds2fed for Windows

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\intel_win

erase *.obj *.exe
make -j 4 SHELL="%ComSpec%" -f ..\Makefile intel_win