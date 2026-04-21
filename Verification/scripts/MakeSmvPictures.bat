@echo off
setlocal

set CURDIR=%CD%
cd ..\..\..
set GITROOT=%CD%
cd %GITROOT%\smv\Verification\Visualization
set VISDIR=%CD%

set SMVBASE=smokeview_win.exe
set SMVEXE=%GITROOT%\smv\Build\smokeview\intel_win\%SMVBASE%
set FDS2FEDBASE=fds2fed_win.exe
set FDS2FEDEXE=%GITROOT%\smv\Build\fds2fed\intel_win\%FDS2FEDBASE%

cd %VISDIR%
call %FDS2FEDEXE% thouse5
call %FDS2FEDEXE% plume5c
call %FDS2FEDEXE% mplume5c8

cd %VISDIR%
call %CURDIR%\SmvCases %SMVEXE% smv

cd %CURDIR%
