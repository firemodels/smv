@echo off
set exe=%1
set option=%2
setlocal

cd ..
set verdir=%CD%

set run=call :runit %exe% %option%

%run%  Visualization cad_test
%run%  Visualization cell_test
%run%  Visualization colorbar
%run%  Visualization colorbar2
%run%  Visualization color_geom
%run%  Visualization geom1
%run%  Visualization geom2
%run%  Visualization hvac_comp
%run%  Visualization mplume5c8
%run%  Visualization objects_dynamic
%run%  Visualization objects_elem
%run%  Visualization obst_remove
%run%  Visualization objects_static
%run%  Visualization obst_test1
%run%  Visualization obst_test4
%run%  Visualization part_color
%run%  Visualization plume5c
%run%  Visualization script_test
%run%  Visualization sillytexture
%run%  Visualization slicemask
%run%  Visualization smoke_sensor

if %option% == fds timeout /t 120 /nobreak

%run%  Visualization smoke_test
%run%  Visualization smoke_test2
%run%  Visualization smoke_test3
%run%  Visualization smoke1
%run%  Visualization smoke2
%run%  Visualization smokex010
%run%  Visualization smokex020
%run%  Visualization smokex040
%run%  Visualization smokex080
%run%  Visualization smokex160
%run%  Visualization smoke_test_geom
%run%  Visualization sprinkler_many
%run%  Visualization test1
%run%  Visualization test2
%run%  Visualization testsort
%run%  Visualization thouse5
%run%  Visualization tour
%run%  Visualization vcirctest
%run%  Visualization vectorskipx
%run%  Visualization vectorskipy
%run%  Visualization vectorskipz

goto eof

:runit
  set prog=%1
  set option=%2
  set casedir=%3
  set input=%4

  cd %verdir%\%casedir%
  if %option% == fds start "%input%" %prog% %input%.fds
  if %option% == smv %prog% -runscript %input%
  exit /b

:eof