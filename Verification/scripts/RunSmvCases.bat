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

call fdsinit > Nul 2>&1
cd %CURDIR%
call SMVCases %FDSEXE% fds

cd %GITROOT%\fds
git describe > %CURDIR%\fds_revision.txt

cd %CURDIR%
%FDSEXE% > fds.out 2>&1
set GREP=%GITROOT%\smv\Verification\scripts\bin\grep
set GAWK=%GITROOT%\smv\Verification\scripts\bin\gawk
type fds.out | %GREP% Revision | %GREP% -v Date | %GAWK% "{print $3}" > %CURDIR%\fds_version.txt

cd %CURDIR%
