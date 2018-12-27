@echo off
setlocal
call ..\scripts\setopts %*
title Building glut library
erase *.o *.obj libglutwin.a libglutwin.lib

make COMPILER=%COMPILER% SIZE=%SIZE% FILTERC="-D WIN32 -D _WIN32 %minggwparms% " -f ./makefile gnu_win_64
endlocal
