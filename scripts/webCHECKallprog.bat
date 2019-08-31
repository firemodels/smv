@echo
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

set progs=background dem2fds flush hashfile smokediff smokezip wind2fds
set smvprogs=get_time set_path sh2bat timep

if NOT "%platform%" == "windows" goto endif1
  echo after if1
  for %%x in ( %progs% ) do (
    cd %svn_root%\smv\Build\%%x\intel_win_64
    start make_%%x
  ) 
  for %%x in ( %smvprogs% ) do (
    cd %svn_root%\smv\Build\%%x\intel_win_64
    start make_%%x
  ) 
    cd %svn_root%\fds\Utilities\fds2ascii\intel_win_64
    start make_fds2ascii
  ) 
::  call :not_built
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

:: -------------------------------------------------------------------------------
:not_built
:: -------------------------------------------------------------------------------
set count=0
  for %%x in ( %progs% ) do (
    cd %svn_root%\smv\Build\%%x\intel_win_64
    if not exist %%x_win_64.exe set /a count=%count%+1
  ) 
  for %%x in ( %smvprogs% ) do (
    cd %svn_root%\smv\Build\%%x\intel_win_64
    if not exist %%x_win_64.exe set /a count=%count%+1
  )
  for %%x in ( %fdsprogs% ) do (
    cd %svn_root%\fds\Utilities\%%x\intel_win_64
    if not exist %%x_win_64.exe set /a count=%count%+1
  ) 
  echo count=%count%
exit /b 0

:: -------------------------------------------------------------
:wait_until_finished
:: -------------------------------------------------------------
Timeout /t 30 >nul 
:loop1
:: FDSBASE defined in Run_SMV_Cases and Run_FDS_Cases (the same in each)
tasklist | grep -ic %FDSBASE% > %WAIT_FILE%
set /p numexe=<%WAIT_FILE%
echo Number of cases running - %numexe%
if %numexe% == 0 goto finished
Timeout /t 30 >nul 
goto loop1
:finished

:eof
echo.
cd %CURDIR%
