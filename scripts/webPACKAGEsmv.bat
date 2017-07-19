@echo off
set platform=%1
set buildtype=%2

:: batch file to generate Windows, Linux or OSX smokeview bundles

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

%svn_drive%

set type=
if "%buildtype%" == "test" (
   set type=test
   set version=%smv_revision%
)
if "%buildtype%" == "release" (
   set type=
   set version=%smv_version%
)

echo.
echo  Bundling %type% Smokeview for %platform%
Title Bundling %type% Smokeview for %platform%

:: windows

if "%platform%" == "windows" (
  call %svn_root%\smv\scripts\batMAKE%type%distwingen 64
  goto eof
)

cd %svn_root%\smv\scripts

set scriptdir=%linux_svn_root%/smv/scripts
set bundledir=%linux_svn_root%/smv/uploads

:: linux

if "%platform%" == "linux" (

  echo.
  echo --- making 64 bit Linux Smokeview installer ---
  echo.
  plink %linux_logon% %scriptdir%/MAKEsmvdist.sh %buildtype% %version% %linux_svn_root% %linux_hostname% %linux_svn_root%

  echo.
  echo --- downloading installer ---
  echo.

  if "%buildtype%" == "release" (
    pscp %linux_logon%:%bundledir%/smv_%version%_linux64.sh   ..\uploads\.
    pscp %linux_logon%:%bundledir%/smv_%version%_linux64.sha1 ..\uploads\.
  )
  if "%buildtype%" == "test" (
    pscp %linux_logon%:%bundledir%/%version%_linux64.sh   ..\uploads\.
    pscp %linux_logon%:%bundledir%/%version%_linux64.sha1 ..\uploads\.
  )
  goto eof
)

:: osx

if "%platform%" == "osx" (
  echo.
  echo --- making 64 bit OSX Smokeview installer ---
  echo.
  plink %osx_logon% %scriptdir%/MAKEsmvdist.sh %buildtype%  %version% %linux_svn_root% %osx_hostname% %linux_svn_root%

  echo.
  echo --- downloading installer ---
  echo.

  if "%buildtype%" == "release" (
    pscp %osx_logon%:%bundledir%/smv_%version%_osx64.sh   ..\uploads\.
    pscp %osx_logon%:%bundledir%/smv_%version%_osx64.sha1 ..\uploads\.
  )
  if "%buildtype%" == "test" (
    pscp %osx_logon%:%bundledir%/%version%_osx64.sh   ..\uploads\.
    pscp %osx_logon%:%bundledir%/%version%_osx64.sha1 ..\uploads\.
  )
  goto eof
)

:eof
echo.
echo Bundle build complete
pause
