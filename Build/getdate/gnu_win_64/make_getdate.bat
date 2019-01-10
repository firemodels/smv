@echo off

Title Building getdate for 64 bit Windows

erase *.o *.exe
make SHELL="%ComSpec%" -f ..\Makefile gnu_win_64
pause


