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
echo Show revisions for the Windows repository, %svn_root%
%svn_drive%
cd %svn_root%\fds
echo repo:%svn_root%\fds
git describe --dirty
echo.

cd %svn_root%\smv
echo repo:%svn_root%\smv
git describe --dirty
echo.

cd %svn_root%\webpages
echo repo:%svn_root%\webpages
git describe --dirty
echo.

set scriptdir=%linux_svn_root%/fds/Utilities/Scripts/
set linux_fdsdir=%linux_svn_root%

echo.
echo ------------------------------------------------------------------------
echo Show revisision for %linux_svn_root%/fds, on %linux_hostname%
plink %linux_logon% %scriptdir%/showrevision.sh  %linux_svn_root%/fds %linux_hostname%
echo.

echo Show revisision for %linux_svn_root%/smv, on %linux_hostname%
plink %linux_logon% %scriptdir%/showrevision.sh  %linux_svn_root%/smv %linux_hostname%
echo.

echo Show revisision for %linux_svn_root%/webpages, on %linux_hostname%
plink %linux_logon% %scriptdir%/showrevision.sh  %linux_svn_root%/webpages %linux_hostname%
echo.

echo.
echo ------------------------------------------------------------------------
echo Show revisision for %linux_svn_root%/fds, on %osx_hostname%
plink %osx_logon% %scriptdir%/showrevision_onhost.sh  %linux_svn_root%/fds %osx_hostname%
echo.

echo Show revisision for %linux_svn_root%/smv, on %osx_hostname%
plink %osx_logon% %scriptdir%/showrevision_onhost.sh  %linux_svn_root%/smv %osx_hostname%
echo.

echo Show revisision for %linux_svn_root%/webpages, on %osx_hostname%
plink %osx_logon% %scriptdir%/showrevision_onhost.sh  %linux_svn_root%/webpages %osx_hostname%
echo.
pause
