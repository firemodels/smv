@echo off
set chid=%1
set casename=%chid%.fds
if exist %chid%.stop erase %chid%.stop
if exist %chid%.running echo %casename% already running
if exist %chid%.running exit /b
echo 1 > %chid%.running
fds %casename%
if exist %chid%.running erase %chid%.running
