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
echo Cleaning %svn_root%\smv\Build
cd %svn_root%\smv\Build
git clean -dxf
echo Cleaning %svn_root%\smv\source
cd %svn_root%\smv\source
git clean -dxf

set scriptdir=%linux_svn_root%/fds/Utilities/Scripts/

echo.
echo ------------------------------------------------------------------------
echo Cleaning source and build directories in the Linux repository %linux_svn_root%, on %linux_hostname%
plink %linux_logon% %scriptdir%/clean_repo_sourcebuild.sh  %linux_svn_root%/fds %linux_hostname%

echo.
echo ------------------------------------------------------------------------
echo Cleaning source and build directories in the OSX repository %linux_svn_root%/fds, on %osx_hostname%
plink %osx_logon% %scriptdir%/clean_repo_sourcebuild.sh

pause
