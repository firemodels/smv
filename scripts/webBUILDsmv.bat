@echo off
set platform=%1
set buildtype=%2
set inc=full

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

set type=
if "%buildtype%" == "test" (
   set type=-t
)
if "%buildtype%" == "testinc" (
   set type=-t
   set inc=inc
)
if "%buildtype%" == "release" (
   set type=-r
)

if "%platform%" == "windows" (
  cd %svn_root%\smv\Build\smokeview\intel_win_64
  call make_smokeview %type% web %inc% glut icon
  goto eof
)
if "%platform%" == "linux" (
  plink %plink_options% %linux_logon% %linux_svn_root%/smv/scripts/run_command.sh smv/Build/smokeview/intel_linux_64 make_smokeview.sh %type%
  goto eof
)
if "%platform%" == "osx" (
  plink %plink_options% %osx_logon% %linux_svn_root%/smv/scripts/run_command.sh smv/Build/smokeview/intel_osx_64 make_smokeview.sh %type%
  goto eof
)

:eof
echo.
echo compilation complete
pause
