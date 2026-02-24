@echo off

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_compilers.bat clang

Title Building fds2fed for Windows

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\clang_win

erase *.obj *.exe
make -j 4 SHELL="%ComSpec%" -f ..\Makefile clang_win