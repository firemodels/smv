@echo off
setlocal

set CURDIR=%CD%
cd ..\..\..
set GITROOT=%CD%
cd %GITROOT%\smv\Verification\Visualization
set VISDIR=%CD%

set FDSBASE=fds_impi_intel_win.exe
set FDSDIR=%GITROOT%\fds\Build\impi_intel_win
set FDSEXE=%FDSDIR%\%FDSBASE%
if exist %FDSEXE% goto skip1
  cd %FDSDIR%
  call make_fds
:skip1

cd %VISDIR%
call fdsinit
call %CURDIR%\SmvCases %FDSEXE% fds
cd %CURDIR%
