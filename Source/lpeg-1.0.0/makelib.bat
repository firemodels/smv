@echo off
setlocal
erase *.o *.lib *.dll
make lpeg.dll CC=icx RANLIB=dir
exit
endlocal
