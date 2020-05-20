@echo off
:: generate terrain with &GEOM
:: set option=-geom

set dem2fds=..\intel_win_64\dem2fds_win_64.exe
set dem2fds=dem2fds

set "GOOGLE=%userprofile%\Google Drive\terrain"
set HOME=%userprofile%\terrain

set "DIR=%GOOGLE%"
if exist %HOME% set "DIR=%HOME%"

%dem2fds% %option% -show -geom -dir "%DIR%\example1" example1.in 
%dem2fds% %option% -geom       -dir "%DIR%\example1" example1a.in 
