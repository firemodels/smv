@echo off
setlocal

set CURDIR=%CD%
cd ..\..\..
set GITROOT=%CD%

cd %CURDIR%
call FDSCases check fds

cd %CURDIR%
