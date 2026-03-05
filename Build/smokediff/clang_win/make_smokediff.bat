@echo off

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_compilers.bat clang

Title Building smokediff for Windows

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\clang_win

set SMV_TESTFLAG=

erase *.obj *.exe
make -j 4 SHELL="%ComSpec%" SMV_TESTFLAG="%SMV_TESTFLAG%"  -f ..\Makefile clang_win
