@echo off

call ..\..\scripts\set_smv_opts %*

:: setup compiler environment
if x%from% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

make GLUT="%GLUT%" SHELL="%ComSpec%" -f ..\Makefile intel_win_64%debug%

