@echo off

:: batch file used to update Windows, Linux and OSX GIT repos

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

:: location of batch files used to set up Intel compilation environment

call %envfile%

echo.
echo ------------------------------------------------------------------------
%svn_drive%

set CURDIR=%CD%

echo Cleaning %svn_root%\fds\Source
cd %svn_root%\fds\Source
git clean -dxf

echo Cleaning %svn_root%\fds\Build
cd %svn_root%\fds\Build
git clean -dxf

echo Cleaning %svn_root%\smv\Source
cd %svn_root%\smv\Source
git clean -dxf

echo Cleaning %svn_root%\smv\Build
cd %svn_root%\smv\Build
git clean -dxf

echo Cleaning %userprofile%\.bundle\uploads
cd %userprofile%\.bundle
rmdir /S /Q uploads
mkdir uploads
 
cd %CURDIR%

pause
