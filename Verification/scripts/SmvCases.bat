@echo off
set exe=%1
set option=%2
setlocal

set run=call :runit %exe% %option%

%run% thouse5
%run% plume5c
%run% mplume5c8

%run% cell_test
%run% colorbar
%run% color_geom
%run% objects_dynamic
%run% objects_static
%run% part_color
%run% sillytexture
%run% slicemask
%run% smoke_test
%run% smoke_test2
%run% smoke_test3
%run% smoke1
%run% smoke2
%run% sphere_propanec
%run% sprinkler_many
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