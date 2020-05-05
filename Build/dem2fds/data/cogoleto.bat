@echo off
:: generate terrain with &GEOM
set option=-geom

set dem2fds=..\intel_win_64\dem2fds_win_64.exe
::set dem2fds=dem2fds

set "GOOGLE=%userprofile%\Google Drive\terrain"
set HOME=%userprofile%\terrain

set "DIR=%GOOGLE%"
if exist %HOME% set "DIR=%HOME%"

%dem2fds% %option% -dir "%DIR%\cogoleto" cogoleto1.in 
