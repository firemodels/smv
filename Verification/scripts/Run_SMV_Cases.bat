@echo off
setlocal

set CURDIR=%CD%
cd ..\..\..
set GITROOT=%CD%
cd %GITROOT%\smv\Verification\Visualization
set VISDIR=%CD%



set FDSBASE=fds_impi_intel_win.exe
set FDSEXE=%GITROOT%\fds\Build\impi_intel_win%DEBUG%\%FDSBASE%

call fdsinit

cd %VISDIR%
start %FDSEXE% thouse5.fds
start %FDSEXE% plume5c.fds
start %FDSEXE% mplume5c8.fds

cd %CURDIR%
