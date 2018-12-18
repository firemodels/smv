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
echo.
%svn_drive%
cd %svn_root%\fds
echo | set /p=Windows: 
git describe --dirty

set scriptdir=%linux_svn_root%/smv/scripts/
set linux_fdsdir=%linux_svn_root%

echo.
echo | set /p=Linux:   
plink %linux_logon% %scriptdir%/showrevision.sh  %linux_svn_root%/fds %linux_hostname%
echo.

echo | set /p=OSX:     
plink %osx_logon% %scriptdir%/showrevision_onhost.sh  %linux_svn_root%/fds %osx_hostname%
echo.


echo.
echo ---------------------------*** smv ***--------------------------------
echo.
cd %svn_root%\smv
echo | set /p=Windows: 
git describe --dirty

echo.
echo | set /p=Linux:   
plink %linux_logon% %scriptdir%/showrevision.sh  %linux_svn_root%/smv %linux_hostname%

echo.
echo | set /p=OSX:     
plink %osx_logon% %scriptdir%/showrevision_onhost.sh  %linux_svn_root%/smv %osx_hostname%
echo.

echo.
echo ---------------------------*** bot ***--------------------------------
echo.
cd %svn_root%\bot
echo | set /p=Windows: 
git describe --dirty

echo.
echo | set /p=Linux:   
plink %linux_logon% %scriptdir%/showrevision.sh  %linux_svn_root%/bot %linux_hostname%

echo.
echo | set /p=OSX:     
plink %osx_logon% %scriptdir%/showrevision_onhost.sh  %linux_svn_root%/bot %osx_hostname%
echo.

echo.
echo ---------------------------*** web ***--------------------------------
echo.
cd %svn_root%\webpages
echo | set /p=Windows: 
git describe --dirty

echo.
echo | set /p=Linux:   
plink %linux_logon% %scriptdir%/showrevision.sh  %linux_svn_root%/webpages %linux_hostname%
echo.

echo | set /p=OSX:     
plink %osx_logon% %scriptdir%/showrevision_onhost.sh  %linux_svn_root%/webpages %osx_hostname%
echo.
pause
