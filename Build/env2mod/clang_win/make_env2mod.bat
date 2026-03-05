@echo off
setlocal
call ..\..\scripts\set_smv_opts %*

Title Building icx windows env2mod

if NOT x%GLUT% == xfreeglut set GLUT=glut

if not x%inc% == xinc erase *.obj *.exe 2> Nul

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\clang_win %GLUT%

:: setup compiler environment
if not defined ONEAPI_ROOT call ..\..\..\Utilities\Scripts\setup_compilers.bat intel

make SHELL="%ComSpec%" -f ..\Makefile clang_win
