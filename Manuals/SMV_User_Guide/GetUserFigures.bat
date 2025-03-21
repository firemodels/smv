@echo off

set CURDIR=%CD%
cd ..\scripts
call GetFigures.sh -s -u
cd %CURDIR%
