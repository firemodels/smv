@echo off

Title Building getdate for 64 bit Windows

erase *.obj *.exe
make SHELL="%ComSpec%" -f ..\Makefile clang_win_64
pause


