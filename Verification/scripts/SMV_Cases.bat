@echo off
set exe=%1
set option=%2
setlocal

set run=call :runit %exe% %option%

%run% cad_test
%run% cell_test
%run% colorbar
%run% colorbar2

%run% color_geom
%run% geom1
%run% geom2
%run% hvac_comp
%run% mplume5c8
%run% objects_dynamic
%run% objects_elem
%run% obst_remove
%run% objects_static
%run% obst_test1
%run% obst_test4
%run% part_color
%run% plume5c
%run% script_test
%run% sillytexture
%run% slicemask
%run% smoke_sensor
%run% smoke_test
%run% smoke_test2
%run% smoke_test3
%run% smoke1
%run% smoke2
%run% smokex010
%run% smokex020
%run% smokex040
%run% smokex080
%run% smokex160
%run% smoke_test_geom
%run% sprinkler_many
%run% test1
%run% test2
%run% testsort
%run% thouse5
%run% tour
%run% vcirctest
%run% vectorskipx
%run% vectorskipy
%run% vectorskipz
goto eof

:runit
  set prog=%1
  set option=%2
  set input=%3

  if %option% == fds start "%input%" %prog% %input%.fds
  if %option% == smv %prog% -runscript %input%
  exit /b

:eof