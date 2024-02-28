@echo off
call %GITROOT%\fds\Utilities\Scripts\getopts.bat %*

echo 2 > %dir%\%infile%.stop
