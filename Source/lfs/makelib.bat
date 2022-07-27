@echo off
setlocal
cd luafilesystem-1_8_0
make -f Makefile.win clean
make CC=icx -f Makefile.win lib
exit
endlocal
