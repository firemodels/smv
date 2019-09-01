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
echo %platform% smokeview utilities build check
echo.
Title %platform% smokeview utilities build check

%svn_drive%

set progs=background dem2fds flush hashfile smokediff smokezip wind2fds
set smvprogs=get_time set_path sh2bat timep

set status=none
if NOT "%platform%" == "windows" goto endif1
  for %%x in ( %progs% ) do (
    cd %svn_root%\smv\Build\%%x\intel_win_64
    if not exist %%x_win_64.exe echo %%x_win_64.exe not built
    if not exist %%x_win_64.exe set status=some
  ) 
  for %%x in ( %smvprogs% ) do (
    cd %svn_root%\smv\Build\%%x\intel_win_64
    if not exist %%x_win_64.exe echo %%x_win_64.exe not built
    if not exist %%x_win_64.exe set status=some
  )
  for %%x in ( %fdsprogs% ) do (
    cd %svn_root%\fds\Utilities\%%x\intel_win_64
    if not exist %%x_win_64.exe echo %%x_win_64.exe not built
    if not exist %%x_win_64.exe set status=some
  ) 
  if "%status%" == "none" echo programs not built: %status%
  goto eof
:endif1

if NOT "%platform%" == "linux" goto endif2
  for %%x in ( %progs% ) do (
    start plink %plink_options% %linux_logon% %linux_svn_root%/smv/scripts/run_command.sh smv/Build/%%x/intel_linux_64 make_%%x.sh
  )
  start plink %plink_options% %linux_logon% %linux_svn_root%/smv/scripts/run_command.sh fds/Utilities/fds2ascii/intel_linux_64 make_fds2ascii.sh
  pause
  goto eof
:endif2

if NOT "%platform%" == "osx" goto endif3
  for %%x in ( %progs% ) do (
    start plink %plink_options% %linux_logon% %linux_svn_root%/smv/scripts/run_command.sh smv/Build/%%x/intel_osx_64 make_%%x.sh
  )
  start plink %plink_options% %linux_logon% %linux_svn_root%/smv/scripts/run_command.sh fds/Utilities/fds2ascii/intel_osx_64 make_fds2ascii.sh
  pause
  goto eof
:endif3
goto eof

:eof
echo.
cd %CURDIR%
pause
