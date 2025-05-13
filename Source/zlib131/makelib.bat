@echo off
setlocal
call ..\scripts\setopts %*
title Building windows zlib library
erase *.o *.obj libz.a libz.lib
set target=libz.lib
if %COMPILER% == gcc set target=libz.a
if exist finished erase finished
make COMPILER=%COMPILER% SIZE=%SIZE% -f ./makefile %target%
if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
echo finished > finished
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
endlocal
