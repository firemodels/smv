@echo off

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_compilers.bat clang

Title Building timep for Windows

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\clang_win

erase *.obj *.exe
make SHELL="%ComSpec%" -f ..\Makefile clang_win