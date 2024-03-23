@echo off

call %GITROOT%\fds\Utilities\Scripts\getopts.bat %*

if exist %dir%\%infile%.stop erase %dir%\%infile%.stop