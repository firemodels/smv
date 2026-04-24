@echo off
setlocal

set CURDIR=%CD%
cd ..\..\..
set GITROOT=%CD%

set FDSBASE=fds_impi_intel_win.exe
set FDSDIR=%GITROOT%\fds\Build\impi_intel_win
set FDSEXE=%FDSDIR%\%FDSBASE%
if exist %FDSEXE% goto skip1
  cd %FDSDIR%
  call make_fds
:skip1

call fdsinit
cd %CURDIR%
call SMV_Cases %FDSEXE% fds
cd %CURDIR%
