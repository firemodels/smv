@echo off

Title Building makepo for 64 bit Windows

erase *.obj *.exe
make -j 4 SHELL="%ComSpec%" -f ..\Makefile clang_win_64
pause