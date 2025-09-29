@echo off

Title Building set_path for 64 bit Windows
erase *.obj *.exe
make SHELL="%ComSpec%" -f ../Makefile clang_win_64
pause
