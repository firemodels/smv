@echo off
set platform=%1

:: batch file for creating libraries on windows, linux or osx

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

echo Building Libraries for %platform%
%svn_drive%
set curdir=%CD%

call %envfile%

:: windows

if "%platform%" == "windows" (
  title building libraries for windows
  cd %svn_root%\smv\Build\LIBS\intel_win_64
  make_LIBS
  goto eof
)

:: osx

if "%platform%" == "osx" (
  title building libraries for osx
  start "clean osx libraries" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh clean
  start "build osx glui" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh glui
  start "build osx zlib" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh zlib
  start "build osx jpeg" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh jpeg
  start "build osx png"  plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh png
  start "build osx gd"   plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh gd
  goto eof
)

:: linux

if "%platform%" == "linux" (
  title building libraries for linux
  start "clean linux libraries" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh clean
  start "build linux glut" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh glut
  start "build linux glui" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh glui
  start "build linux zlib" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh zlib
  start "build linux jpeg" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh jpeg
  start "build linux png"  plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh png
  start "build linux gd"   plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh gd
  goto eof
)


:eof
cd %curdir%
pause
