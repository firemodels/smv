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
  title building libraries for windows using the Intel compilers
  cd %svn_root%\smv\Build\LIBS\intel_win_64
  make_LIBS
  goto eof
)

if "%platform%" == "windowsgnu" (
  title building libraries for windows using the gnu compilers
  cd %svn_root%\smv\Build\LIBS\gnu_win_64
  make_LIBS
  goto eof
)

:: gnuosx

if "%platform%" == "gnuosx" (
  title building libraries for osx using gnu
  start /wait "cleaning osx libraries"    plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/gnu_osx_64/make_LIBS.sh -t clean
  start "building osx glui library" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/gnu_osx_64/make_LIBS.sh -t glui
  start "building osx zlib library" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/gnu_osx_64/make_LIBS.sh -t zlib
  start "building osx jpeg library" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/gnu_osx_64/make_LIBS.sh -t jpeg
  start "building osx png library"  plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/gnu_osx_64/make_LIBS.sh -t png
  start "building osx gd library"   plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/gnu_osx_64/make_LIBS.sh -t gd
  goto eof
)

:: osx

if "%platform%" == "osxquartz" (
  title building libraries for osx - Quartz
  start /wait "cleaning osx libraries" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_q_64/make_LIBS.sh -t clean
  start "building osx glut library" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_q_64/make_LIBS.sh -t glut
  start "building osx glui library" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_q_64/make_LIBS.sh -t glui
  start "building osx zlib library" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_q_64/make_LIBS.sh -t zlib
  start "building osx jpeg library" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_q_64/make_LIBS.sh -t jpeg
  start "building osx png library"  plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_q_64/make_LIBS.sh -t png
  start "building osx gd library"   plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_q_64/make_LIBS.sh -t gd
  goto eof
)

:: osx

if "%platform%" == "osx" (
  title building libraries for osx - non Quartz
  start /wait "cleaning osx libraries" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh -t clean
  start "building osx glui library" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh -Q -t glui
  start "building osx zlib library" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh -Q -t zlib
  start "building osx jpeg library" plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh -Q -t jpeg
  start "building osx png library"  plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh -Q -t png
  start "building osx gd library"   plink %plink_options% %osx_logon% %linux_svn_root%/smv/Build/LIBS/intel_osx_64/make_LIBS.sh -Q -t gd
  goto eof
)

:: gnulinux

if "%platform%" == "gnulinux" (
  title building libraries for linux using gnu
  start /wait "cleaning linux libraries" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/gnu_linux_64/make_LIBS.sh    -t clean
  start "building linux glut library" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/gnu_linux_64/make_LIBS.sh -t glut
  start "building linux glui library" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/gnu_linux_64/make_LIBS.sh -t glui
  start "building linux zlib library" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/gnu_linux_64/make_LIBS.sh -t zlib
  start "building linux jpeg library" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/gnu_linux_64/make_LIBS.sh -t jpeg
  start "building linux png library"  plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/gnu_linux_64/make_LIBS.sh -t png
  start "building linux gd library"   plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/gnu_linux_64/make_LIBS.sh -t gd
  goto eof
)

:: linux

if "%platform%" == "linux" (
  title building libraries for linux
  start /wait "cleaning linux libraries" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh    -t clean
  start "building linux glut library" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh -t glut
  start "building linux glui library" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh -t glui
  start "building linux zlib library" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh -t zlib
  start "building linux jpeg library" plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh -t jpeg
  start "building linux png library"  plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh -t png
  start "building linux gd library"   plink %plink_options% %linux_logon% %linux_svn_root%/smv/Build/LIBS/intel_linux_64/make_LIBS.sh -t gd
  goto eof
)


:eof
cd %curdir%
pause
