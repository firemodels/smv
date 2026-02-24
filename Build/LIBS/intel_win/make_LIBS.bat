@echo off
set freeglut=
if %1 == f set freeglut=clang_win
call ..\..\..\Source\scripts\setopts.bat i
call ..\make_LIBS_common.bat i %freeglut%
