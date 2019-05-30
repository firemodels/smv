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
echo ---------------------------*** fds ***--------------------------------
%svn_drive%
cd %svn_root%\fds
echo Windows
git checkout master

set scriptdir=%linux_svn_root%/smv/scripts/
set linux_fdsdir=%linux_svn_root%

echo.
echo Linux
plink %plink_options% %linux_logon% %scriptdir%/setbranch.sh  %linux_svn_root%/fds
echo.

echo OSX
plink %plink_options% %osx_logon% %scriptdir%/setbranch.sh  %linux_svn_root%/fds


echo.
echo ---------------------------*** smv ***--------------------------------
cd %svn_root%\smv
echo Windows
git checkout master

echo.
echo Linux
plink %plink_options% %linux_logon% %scriptdir%/setbranch.sh  %linux_svn_root%/smv

echo.
echo OSX
plink %plink_options% %osx_logon% %scriptdir%/setbranch.sh  %linux_svn_root%/smv

echo.
echo ---------------------------*** bot ***--------------------------------
cd %svn_root%\bot
echo Windows
git checkout master

echo.
echo Linux
plink %plink_options% %linux_logon% %scriptdir%/setbranch.sh  %linux_svn_root%/bot

echo.
echo OSX
plink %plink_options% %osx_logon% %scriptdir%/setbranch.sh  %linux_svn_root%/bot

echo.
pause
