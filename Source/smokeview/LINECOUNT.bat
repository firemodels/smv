@echo off
cat *.c *.cpp *.h *.f90 ..\shared\*.h ..\shared\*.c ..\shared\mbedtls\*.h | wc -l