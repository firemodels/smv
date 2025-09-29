@echo off

erase *.obj *.exe
make -j 4 SHELL="%ComSpec%" -f ..\Makefile clang_win_64

