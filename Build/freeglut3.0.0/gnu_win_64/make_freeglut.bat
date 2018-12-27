@echo off
setlocal
title Building freeglut library

::erase *.o *.obj libglutwin.a libglutwin.lib
make -f ./makefile gnu_win_64
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
endlocal
