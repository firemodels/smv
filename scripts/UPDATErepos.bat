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
echo directory: %svn_root%
echo host: windows
%svn_drive%
cd %svn_root%\fds
echo.
echo *** fds ***
git remote update
git checkout master
git merge origin/master
git merge firemodels/master
git describe --dirty

cd %svn_root%\smv
echo.
echo *** smv ***
git remote update
git checkout master
git merge origin/master
git merge firemodels/master
git describe --dirty

cd %svn_root%\webpages
echo.
echo *** webpages ***
git checkout nist-pages
git remote update
git merge origin/nist-pages
git describe --dirty


set scriptdir=%linux_svn_root%/fds/Utilities/Scripts/
set linux_fdsdir=%linux_svn_root%

echo.
echo ------------------------------------------------------------------------
echo directory: %linux_svn_root%
echo host: %linux_hostname%
echo.
echo *** fds ***
plink %linux_logon% %scriptdir%/UPDATE_thishost.sh  %linux_svn_root%/fds %linux_hostname%

echo.
echo *** smv ***
plink %linux_logon% %scriptdir%/UPDATE_thishost.sh  %linux_svn_root%/smv %linux_hostname%

echo.
echo *** webpages ***
plink %linux_logon% %scriptdir%/UPDATE_webpages.sh  %linux_svn_root%/webpages %linux_hostname%

echo.
echo ------------------------------------------------------------------------
echo directory: %linux_svn_root%
echo host: %osx_hostname%
echo.
echo *** fds ***
plink %osx_logon% %scriptdir%/UPDATE_latest_fds_onhost.sh  %linux_svn_root%/fds %osx_hostname%

echo.
echo *** smv ***
plink %osx_logon% %scriptdir%/UPDATE_latest_fds_onhost.sh  %linux_svn_root%/smv %osx_hostname%

echo.
echo *** webpages ***
plink %osx_logon% %scriptdir%/UPDATE_webpages_onhost.sh  %linux_svn_root%/webpages %osx_hostname%
pause
