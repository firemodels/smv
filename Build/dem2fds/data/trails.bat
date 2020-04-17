@echo off
:: generate terrain with &GEOM
set option=-geom

set dem2fds=..\intel_win_64\dem2fds_win_64.exe
::set dem2fds=dem2fds

set "GOOGLE=%userprofile%\Google Drive\documents\terrain"
set HOME=%userprofile%\terrain
set "DIR=%GOOGLE%"


::%dem2fds% %option% -show -dir "DIR%"\trails trails.in 
::%dem2fds% %option% -show -dir "DIR%"\trails trails2.in 
%dem2fds% %option%       -dir "DIR%"\trails" trails4.in 
