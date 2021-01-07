@echo off
set platform=%1
set buildtype=%2
set inc=

:: batch file to build test or release smokeview on Windows, Linux or OSX platforms

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
echo.
echo  Building %buildtype% Smokeview for %platform%
Title Building %buildtype% Smokeview for %platform%

%svn_drive%

set wintype=
set type=
set wininc=
set inc=

:: ----------- windows -----------------

if "%buildtype%" == "test" (
   set wintype=-test
   set type=-T
)
if "%buildtype%" == "testinc" (
   set wintype=-test
   set wininc=-inc
   set type=-T
   set inc=-i
)
if "%buildtype%" == "release" (
   set wintype=-release
   set type=-r
)
if "%buildtype%" == "debug" (
   set wintype=
   set type=
)
if "%platform%" == "windowsgnu" (
  cd %svn_root%\smv\Build\smokeview\gnu_win_64
  call make_smokeview -test -profile
  goto eof
)

if "%platform%" == "windows" (
  cd %svn_root%\smv\Build\smokeview\intel_win_64
  call make_smokeview %wintype% %wininc% -glut -icon
  goto eof
)

:: ----------- linux -----------------

if "%platform%" == "linux" (
  plink %plink_options% %linux_logon% %linux_svn_root%/smv/scripts/run_command.sh smv/Build/smokeview/intel_linux_64 make_smokeview.sh %type%
  goto eof
)
if "%platform%" == "linuxgnu" (
  plink %plink_options% %linux_logon% %linux_svn_root%/smv/scripts/run_command.sh smv/Build/smokeview/gnu_linux_64 make_smokeview.sh -p -T
  goto eof
)

:: ----------- osx -----------------

if "%platform%" == "osxquartz" (
  plink %plink_options% %osx_logon% %linux_svn_root%/smv/scripts/run_command.sh smv/Build/smokeview/intel_osx_q_64 make_smokeview.sh %type%
  goto eof
)
if "%platform%" == "osx" (
  plink %plink_options% %osx_logon% %linux_svn_root%/smv/scripts/run_command.sh smv/Build/smokeview/intel_osx_64 make_smokeview.sh %type%
  goto eof
)
if "%platform%" == "osxgnu" (
  plink %plink_options% %osx_logon% %linux_svn_root%/smv/scripts/run_command.sh smv/Build/smokeview/gnu_osx_64 make_smokeview.sh -p -T
  goto eof
)

:eof
echo.
echo compilation complete
pause
