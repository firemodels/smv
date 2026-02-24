@echo off
call ..\..\scripts\set_smv_opts %*

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_compilers.bat clang

Title Building clang debug windows smokeview

if NOT x%GLUT% == xfreeglut set GLUT=glut

if not x%inc% == xinc erase *.obj *.exe 2> Nul

:: build libraries if one is missing
call ..\..\scripts\test_libs.bat ..\..\LIBS\clang_win

make -j %NUMBER_OF_PROCESSORS% ICON="%ICON%" GLUT="%GLUT%" SHELL="%ComSpec%" SMV_TESTFLAG="%SMV_TESTFLAG%" -f ..\Makefile clang_win_db
