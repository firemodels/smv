@echo off
set curdir=%CD%
cd ..\smokeview
cat *.c *.cpp *.h ..\shared\*.h ..\shared\*.c  | wc -l
cd %curdir%