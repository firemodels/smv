@echo off

:: batch file to output git repo revision string

:: setup environment variables (defining where repository resides etc) 

set envfile="%userprofile%"\fds_smv_env.bat
IF EXIST %envfile% GOTO endif_envexist
echo ***Fatal error.  The environment setup file %envfile% does not exist. 
echo Create a file named %envfile% and use smv/scripts/fds_smv_env_template.bat
echo as an example.
echo.
echo Aborting now...
pause>NUL
goto:eof

:endif_envexist

call %envfile%
set bundledir=%userprofile%\.bundle

%svn_drive%
set CURDIR=%CD%
cd %svn_root%\smv
echo.
echo | set /p=smv revision: 
type %bundledir%\smv_revision.txt

cd %svn_root%\fds
cd %CURDIR%
echo | set /p=fds revision: 
type %bundledir%\fds_revision.txt
echo.
pause
