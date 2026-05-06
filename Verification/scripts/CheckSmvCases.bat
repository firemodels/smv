@echo off
setlocal

set CURDIR=%CD%
cd ..\..\..
set GITROOT=%CD%

cd %CURDIR%
call SMVCases check smv

cd %CURDIR%
