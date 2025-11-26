@echo off
set convert=..\..\Build\wind2fds\intel_win\wind2fds_win.exe

if exist %convert% goto convert_wind
echo "***error: The program %convert% does not exist"

goto eof

:convert_wind
copy ..\..\..\fig\smv\wind\a180210.csv a180210.csv 1> Nul 2>&1
%convert% -prefix sd11 -offset "1500.0 1500.0 70.0" a180210.csv

:eof
