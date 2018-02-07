@echo off
cat *.c *.cpp *.h *.f90 ..\shared\*.c ..\shared\*.h ..\background\*.c ..\background\*.h ..\smokediff\*.h ..\smokediff\*.c ..\smokezip\*.c ..\smokezip\*.h ..\smokezip\*.f90 ..\wind2fds\*.c ..\wind2fds\*.h ..\dem2fds\*.c ..\dem2fds\*.h| wc
