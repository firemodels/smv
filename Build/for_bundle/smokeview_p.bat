@echo off
set scriptname=%0

set repo=
set option=run 

call :getopts %*
if %stopscript% == 1 (
  exit /b
)

set mypath=%~dp0

if x%repo% == x goto else1
  set smokeview=%userprofile%\%repo%\smv\Build\smokeview\gnu_win_64\smokeview_win_test_64_db.exe
  if EXIST %smokeview% goto endif1
  echo ***error: %smokeview% does not exist
  echo aborted
  exit /b
:else1
  set smokeview=smokeview_gnu.exe
  call :is_file_installed %smokeview% || exit /b 1
  where %smokeview% | head -1 > %TEMP%\smokeview_path.txt
  set /p smv=<%TEMP%\smokeview_path.txt
  if exist %TEMP%\smokeview_path.txt erase %TEMP%\smokeview_path.txt  
  set "smokeview=%smv%"
:endif1

call :is_file_installed gprof || exit /b 1

if "%option%" == "profile" goto endif2
  "%smokeview%" %casename%
:endif2

gprof "%smokeview%" > %casename%_profile.txt
if exist %casename%_profile.txt notepad %casename%_profile.txt
goto eof

:: -------------------------------------------------------------
:is_file_installed
:: -------------------------------------------------------------

  set program=%1
  set exist=%TEMP%\exist.txt
  set count=%TEMP%\count.txt
  %program% --help 1>> %exist% 2>&1
  type %exist% | find /i /c "not recognized" > %count%
  set /p nothave=<%count%
  if %nothave% == 1 (
    echo "***Fatal error: %program% not present"
    erase %exist%
    erase %count%
    exit /b 1
  )
  erase %exist%
  erase %count%
  exit /b 0

:: -------------------------------------------------------------
:getopts
:: -------------------------------------------------------------
 set stopscript=0
 if (%1)==() exit /b
 set valid=0
 set arg=%1
 set firstchar=%arg:~0,1%
 if NOT "%firstchar%" == "-" goto endloop
 if /I "%1" EQU "-f" (
   set repo=FireModels_fork
   set valid=1
 )
 if /I "%1" EQU "-h" (
   call :usage
   set stopscript=1
   exit /b
 )
 if /I "%1" EQU "-p" (
   set option=profile
   set valid=1
 )
 if /I "%1" EQU "-r" (
   set repo=%2
   set valid=1
   shift
 )
 shift
 if %valid% == 0 (
   echo.
   echo ***Error: the argument %arg% is invalid
   echo.
   echo Usage:
   call :usage
   set stopscript=1
   exit /b
 )
if not (%1)==() goto getopts
:endloop
set casename=%arg%
exit /b

:: -------------------------------------------------------------
:usage
:: -------------------------------------------------------------
echo %scriptname% [options] casename
echo. 
echo This script runs the gnu version of smokeview then profiles it using the
echo gprof profiler.  The smokeview hot spots identified by gprof are output to 
echo the file casename_profile.txt .  Other options are given below.
echo. 
echo -f      - same as -r FireModels_fork
echo -h      - display this message
echo -p      - only profile smokeview_gnu (assume that smokeview_gnu was already run)
echo -r root - use the version of smokeview_gnu found in %userprofile%\root\smv\Build\smokeview\gnu_win_64,
echo           otherwise use the installed version of smokeview_gnu
exit /b

:eof