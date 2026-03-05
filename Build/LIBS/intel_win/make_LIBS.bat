@echo off

set freeglut=
if x%1 == xfreeglut set freeglut=intel_win
call ..\..\..\Source\scripts\setopts.bat i
call ..\make_LIBS_common.bat i %freeglut%
