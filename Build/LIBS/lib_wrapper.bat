@echo off
echo. > %2\%1.running
call  %3 %4 %5 %6 %7 %8 %9
erase %2\%1.running