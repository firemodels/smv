@echo off
setlocal
title Building freeglut library

erase *.o *.obj *.a *.lib
make PLATFORM="win" -f ../Makefile gnu_win_64
endlocal
