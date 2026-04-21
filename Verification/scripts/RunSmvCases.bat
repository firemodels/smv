@echo off
setlocal

set CURDIR=%CD%
cd ..\..\..
set GITROOT=%CD%
cd %GITROOT%\smv\Verification\Visualization
set VISDIR=%CD%



set FDSBASE=fds_impi_intel_win.exe
set FDSEXE=%GITROOT%\fds\Build\impi_intel_win%DEBUG%\%FDSBASE%
set PROG=start %FDSEXE%

call fdsinit

cd %VISDIR%
call %CURDIR%\SmvCases
cd %CURDIR%
