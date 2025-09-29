@echo off

Title Building 64 bit Windows setpath
erase *.o *.exe
make SHELL="%ComSpec%" -f ../Makefile clang_win_64
pause
