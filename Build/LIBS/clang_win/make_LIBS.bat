@echo off
set freeglut=
if %1 == f set freeglut=clang_win
call ..\..\..\Source\scripts\setopts.bat c
call ..\make_LIBS_common.bat c %freeglut%
