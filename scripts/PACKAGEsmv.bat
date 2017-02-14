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
)
if "%buildtype%" == "release" (
   set type=
)

echo.
echo  Bundling %type% Smokeview for %platform%
Title Bundling %type% Smokeview for %platform%

:: windows

if "%platform%" == "windows" (
  call %svn_root%\smv\scripts\MAKE%type%distwingen 64
  goto eof
)

cd %svn_root%\smv\scripts
set version=%smv_version%

set scriptdir=%linux_svn_root%/smv/scripts
set bundledir=%linux_svn_root%/smv/uploads

:: linux

if "%platform%" == "linux" (

  echo.
  echo --- making 64 bit Linux Smokeview installer ---
  echo.
  if "%buildtype%" == "release" (
    plink %linux_logon% %scriptdir%/MAKEdistgen.sh %version% linux %linux_hostname% %fds_edition% %linux_svn_root%
  )
  if "%buildtype%" == "test" (
    plink %linux_logon% %scriptdir%/MAKEtestdistlinux64.sh %smv_revision% %linux_svn_root%
  )

  echo.
  echo --- downloading installer ---
  echo.

  if "%buildtype%" == "release" (
    pscp %linux_logon%:%bundledir%/smv_%version%_linux64.sh ..\uploads\.
    pscp %linux_logon%:%bundledir%/smv_%version%_linux64.sh.md5 ..\uploads\.
  )
  if "%buildtype%" == "test" (
    if exist ..\uploads\%smv_revision%_linux64 rmdir /s /q ..\uploads\%smv_revision%_linux64
    mkdir ..\uploads\%smv_revision%_linux64
    pscp %linux_logon%:%bundledir%/%smv_revision%_linux64.sh ..\uploads\.
    pscp %linux_logon%:%bundledir%/%smv_revision%_linux64.sh.md5 ..\uploads\%smv_revision%_linux64\.
    pscp %linux_logon%:%bundledir%/%smv_revision%_linux64/bin/MD5/*.md5 ..\uploads\%smv_revision%_linux64\.
  )
  goto eof
)

:: osx

if "%platform%" == "osx" (
  echo.
  echo --- making 64 bit OSX Smokeview installer ---
  echo.
  if "%buildtype%" == "release" (
    plink %osx_logon% %scriptdir%/MAKEdistgen.sh %version% osx %osx_hostname% %fds_edition% %linux_svn_root%
  )
  if "%buildtype%" == "test" (
    plink %osx_logon% %scriptdir%/MAKEtestdistosx64.sh %smv_revision% %linux_svn_root% %osx_hostname% %linux_svn_root%
  )

  echo.
  echo --- downloading installer ---
  echo.

  if "%buildtype%" == "release" (
    pscp %osx_logon%:%bundledir%/smv_%version%_osx64.sh ..\uploads\.
    pscp %osx_logon%:%bundledir%/smv_%version%_osx64.sh.md5 ..\uploads\.
  )
  if "%buildtype%" == "test" (
    if exist ..\uploads\%smv_revision%_osx64 rmdir /s /q ..\uploads\%smv_revision%_osx64
    mkdir ..\uploads\%smv_revision%_osx64
    pscp %osx_logon%:%bundledir%/%smv_revision%_osx64.sh ..\uploads\.
    pscp %osx_logon%:%bundledir%/%smv_revision%_osx64.sh.md5 ..\uploads\%smv_revision%_osx64\.
    pscp %osx_logon%:%bundledir%/%smv_revision%_osx64/bin/MD5/*.md5 ..\uploads\%smv_revision%_osx64\.
  )
  goto eof
)

:eof
echo.
echo Bundle build complete
pause
