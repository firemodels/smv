@echo off
:: setup MS compiler environment
call "%VS_COMPILER%\vcvarsall.bat" amd64

Title Building set_path for 64 bit Windows

erase *.obj *.mod
make -f ..\Makefile ms_win_64

