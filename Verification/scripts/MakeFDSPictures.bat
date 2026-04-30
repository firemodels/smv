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

::erase %GITROOT%\smv\Manuals\SMV_User_Guide\SCRIPT_FIGURES\*.png
::erase %GITROOT%\smv\Manuals\SMV_Verification_Guide\SCRIPT_FIGURES\*.png

cd %CURDIR%
call FDS_Cases %SMVEXE% smv
cd %CURDIR%
