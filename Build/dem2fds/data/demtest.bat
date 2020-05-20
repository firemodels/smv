@echo off
:: generate terrain with &GEOM
:: set option=-geom

:: generate terrain with &OBST
set option=-obst

set dem2fds=..\intel_win_64\dem2fds_win_64.exe
::set dem2fds=dem2fds

set "GOOGLE=%userprofile%\Google Drive\terrain"
set HOME=%userprofile%\terrain

set "DIR=%GOOGLE%"
if exist %HOME% set "DIR=%HOME%"

%dem2fds% %option% -show -elevdir "%DIR%\N40W078" -dir "%DIR%\demtest" demtest1.in 
%dem2fds% %option% -show -elevdir "%DIR%\N40W078" -dir "%DIR%\demtest" demtest2.in 

