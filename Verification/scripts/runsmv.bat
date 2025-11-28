@echo off

call %GITROOT%\bot\Bundlebot\nightly\getopts.bat %*

set fulldir=%BASEDIR%/%dir%

cd %fulldir%
echo %infile%
%SMOKEVIEW% -runscript %infile%
