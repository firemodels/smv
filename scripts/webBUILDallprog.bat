@echo off
set platform=%1

set CURDIR=%CD%

:: batch file to build smokeview utility programs on Windows, Linux or OSX platforms

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
echo  Building smokeview utilities for 64 bit %platform%
Title Building smokeview utilities for 64 bit %platform%

%svn_drive%

set EXIT_SCRIPT=1

set progs=background flush hashfile smokediff smokezip wind2fds
set smvprogs=get_time set_path sh2bat timep

if NOT "%platform%" == "windows" goto endif1
  echo after if1
  for %%x in ( %progs% ) do (
    cd %svn_root%\smv\Build\%%x\intel_win_64
    start "building windows %%x" make_%%x
  ) 
  for %%x in ( %smvprogs% ) do (
    cd %svn_root%\smv\Build\%%x\intel_win_64
    start "building windows %%x" make_%%x
  ) 
    cd %svn_root%\fds\Utilities\fds2ascii\intel_win_64
    start "building windows fds2ascii" make_fds2ascii
  ) 
::  call :not_built
  goto eof
:endif1

if NOT "%platform%" == "linux" goto endif2
  for %%x in ( %progs% ) do (
    start "building linux %%x" plink %plink_options% %linux_logon% %linux_svn_root%/smv/scripts/run_command.sh smv/Build/%%x/intel_linux_64 make_%%x.sh
  )
  start "building linux %%x" plink %plink_options% %linux_logon% %linux_svn_root%/smv/scripts/run_command.sh fds/Utilities/fds2ascii/intel_linux_64 make_fds2ascii.sh
  pause
  goto eof
:endif2

if NOT "%platform%" == "osx" goto endif3
  for %%x in ( %progs% ) do (
    start "building osx %%x" plink %plink_options% %osx_logon% %linux_svn_root%/smv/scripts/run_command.sh smv/Build/%%x/intel_osx_64 make_%%x.sh
  )
  start "building osx fds2ascii" plink %plink_options% %osx_logon% %linux_svn_root%/smv/scripts/run_command.sh fds/Utilities/fds2ascii/intel_osx_64 make_fds2ascii.sh
  pause
  goto eof
:endif3
goto eof

:eof
echo.
cd %CURDIR%
