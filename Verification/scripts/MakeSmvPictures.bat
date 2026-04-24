@echo off
setlocal

set CURDIR=%CD%
cd ..\..\..
set GITROOT=%CD%

set SMVBASE=smokeview_win.exe
set SMVDIR=%GITROOT%\smv\Build\smokeview\intel_win
set SMVEXE=%SMVDIR%\%SMVBASE%
if exist %SMVEXE% goto skip1
  cd %SMVDIR%
  call make_smokeview
:skip1

set FDS2FEDBASE=fds2fed_win.exe
set FDS2FEDDIR=%GITROOT%\smv\Build\fds2fed\intel_win\
set FDS2FEDEXE=%FDS2FEDDIR%\%FDS2FEDBASE%
if exist %FDS2FEDEXE% goto skip2
  cd %FDS2FEDDIR%
  call make_fds2fed
:skip2

cd %GITROOT%\smv\Verification\Visualization
call %FDS2FEDEXE% thouse5
call %FDS2FEDEXE% plume5c
call %FDS2FEDEXE% mplume5c8

erase %GITROOT%\smv\Manuals\SMV_User_Guide\SCRIPT_FIGURES\*.png
erase %GITROOT%\smv\Manuals\SMV_Verification_Guide\SCRIPT_FIGURES\*.png

cd %CURDIR%
call SMV_Cases %SMVEXE% smv
cd %CURDIR%
