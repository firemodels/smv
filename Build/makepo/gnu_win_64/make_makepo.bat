@echo off

Title Building makepo for 64 bit Windows

erase *.o *.exe
make -j 4 SHELL="%ComSpec%" -f ..\Makefile gnu_win_64
pause