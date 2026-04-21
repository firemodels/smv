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
%run% part_color
%run% slicemask
%run% sprinkler_many
%run% vcirctest
goto eof

:runit
  set prog=%1
  set option=%2
  set input=%3

  if %option% == fds start "%input%" %prog% %input%.fds
  if %option% == smv %prog% -runscript %input%
  exit /b

:eof