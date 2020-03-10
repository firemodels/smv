@echo off
set platform=%1
set buildtype=%2

::  batch file to build test or release Smokeview on a Windows, OSX or Linux system

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
if "%buildtype%" == "test" (
  echo.
  echo  Installing test %platform% Smokeview
  Title  Installing test %platform% Smokeview
)
if "%buildtype%" == "release" (
  echo.
  echo  Installing %platform% Smokeview
  Title  Installing %platform% Smokeview
)

%svn_drive%

if "%platform%" == "windows" (
  cd %userprofile%\.bundle\uploads
  if "%buildtype%" == "test" (
    echo Running Smokeview installer:  %smv_revision%_win.exe
    call %smv_revision%_win.exe
  )
  if "%buildtype%" == "release" (
    echo Running Smokeview installer: %smv_version%_win.exe
    call %smv_version%_win.exe
  )
  goto eof
)
if "%platform%" == "linux" (
  if "%buildtype%" == "test" (
    plink %plink_options% %linux_logon% %linux_svn_root%/smv/scripts/run_command2.sh $HOME/.bundle/uploads %smv_revision%_lnx.sh y
  )
  if "%buildtype%" == "release" (
    plink %plink_options% %linux_logon% %linux_svn_root%/smv/scripts/run_command2.sh $HOME/.bundle/uploads %smv_version%_lnx.sh y
  )
  goto eof
)
if "%platform%" == "osx" (
  if "%buildtype%" == "test" (
    plink %plink_options% %osx_logon% %linux_svn_root%/smv/scripts/run_command2.sh $HOME/.bundle/uploads %smv_revision%_osx.sh y
  )
  if "%buildtype%" == "release" (
    plink %plink_options% %osx_logon% %linux_svn_root%/smv/scripts/run_command2.sh $HOME/.bundle/uploads %smv_version%_osx.sh y
  )
  goto eof
)

:eof
echo.
echo compilation complete
pause
