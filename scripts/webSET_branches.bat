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
echo *** fds ***
%svn_drive%
cd %svn_root%\fds
echo Windows
git checkout master

set scriptdir=%linux_svn_root%/smv/scripts/
set linux_fdsdir=%linux_svn_root%

echo.
echo Linux
plink %linux_logon% %scriptdir%/setbranch.sh  %linux_svn_root%/fds %linux_hostname%
echo.

echo OSX
plink %osx_logon% %scriptdir%/setbranch_onhost.sh  %linux_svn_root%/fds %osx_hostname%
echo.


echo.
echo ------------------------------------------------------------------------
echo *** smv ***
cd %svn_root%\smv
echo Windows
git checkout master

echo.
echo Linux
plink %linux_logon% %scriptdir%/setbranch.sh  %linux_svn_root%/smv %linux_hostname%

echo.
echo OSX
plink %osx_logon% %scriptdir%/setbranch_onhost.sh  %linux_svn_root%/smv %osx_hostname%
echo.
echo.
pause
