@echo off

call ..\..\scripts\set_smv_opts %*

make GLUT="%GLUT%" SHELL="%ComSpec%" -f ..\Makefile clang_win_64

