@echo off
cat *.c *.cpp *.h *.f90 ..\shared\*.h ..\shared\*.c  | wc -l