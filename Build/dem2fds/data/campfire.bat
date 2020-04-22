@echo off
:: generate terrain with &GEOM
set option=-geom

set dem2fds=..\intel_win_64\dem2fds_win_64.exe
::set dem2fds=dem2fds

set "GOOGLE=%userprofile%\Google Drive\documents\terrain"
set HOME=%userprofile%\terrain
set "DIR=%GOOGLE%"

set terrain=
if EXIST "%terrain1%" set "terrain=%terrain1%"
if EXIST "%terrain2%" set "terrain=%terrain2%"
if NOT "x%terrain%" == "x" goto endif1
  echo ***error: the terrain file directory does not exist
  exit /b
:endif1

%dem2fds% %option%  -width 3000 -dir "%DIR%\campfire" campfire3.in 
::%dem2fds% %option% -show      -dir "%DIR%\campfire" campfire4.in 
