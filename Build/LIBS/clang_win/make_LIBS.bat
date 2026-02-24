@echo off

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_compilers.bat clang

set freeglut=
if x%1 == xfreeglut set freeglut=clang_win
call ..\..\..\Source\scripts\setopts.bat c
call ..\make_LIBS_common.bat c %freeglut%
