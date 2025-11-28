@echo off
call %GITROOT%\bot\Bundlebot\nightly\getopts.bat %*

echo 2 > %dir%\%infile%.stop
