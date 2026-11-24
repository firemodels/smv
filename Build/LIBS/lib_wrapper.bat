@echo off
call %3 %4 %5 %6 %7 %8 %9 > "%~2\%~1.out" 2>&1
erase "%~2\%~1.running"
