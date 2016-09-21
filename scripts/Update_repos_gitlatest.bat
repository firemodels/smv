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
echo Updating the Windows repository, %svn_root%, to the latest revision
%svn_drive%
cd %svn_root%\fds
echo Updating the repo:%svn_root%\fds
git remote update
git checkout master
git merge origin/master
git merge firemodels/master
git describe --dirty

cd %svn_root%\smv
echo Updating the repo:%svn_root%\smv
git remote update
git checkout master
git merge origin/master
git merge firemodels/master
git describe --dirty

cd %svn_root%\webpages
echo Updating the repo:%svn_root%\webpages
git checkout nist-pages
git remote update
git merge origin/nist-pages
git describe --dirty


set scriptdir=%linux_svn_root%/fds/Utilities/Scripts/
set linux_fdsdir=%linux_svn_root%

echo.
echo ------------------------------------------------------------------------
echo Updating the Linux GIT repository, %linux_svn_root%/fds, on %linux_hostname% to the latest revision
plink %linux_logon% %scriptdir%/UPDATE_thishost.sh  %linux_svn_root%/fds %linux_hostname%

echo Updating the Linux GIT repository, %linux_svn_root%/smv, on %linux_hostname% to the latest revision
plink %linux_logon% %scriptdir%/UPDATE_thishost.sh  %linux_svn_root%/smv %linux_hostname%

echo.
echo ------------------------------------------------------------------------
echo Updating the OSX GIT repository, %linux_svn_root%/fds, on %osx_hostname% to the latest revision
plink %osx_logon% %scriptdir%/UPDATE_latest_fds_onhost.sh  %linux_svn_root%/fds %osx_hostname%

echo Updating the OSX GIT repository, %linux_svn_root%/smv, on %osx_hostname% to the latest revision
plink %osx_logon% %scriptdir%/UPDATE_latest_fds_onhost.sh  %linux_svn_root%/smv %osx_hostname%
pause
