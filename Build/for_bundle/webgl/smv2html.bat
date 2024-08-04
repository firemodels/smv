@echo off
:: usage: 
::  smv2html -host hostname -casedir casedir -renderdir renderdir casename

set stopscript=0
set showcommand=0
set exe=smokeview
set renderdir=.
set hostname=
set casedir=.

set websetup=%userprofile%\web_setup.bat
if not exist %websetup% goto skip_setup
  call %websetup%
:skip_setup

call :getopts %*
if %stopscript% == 1 (
  exit /b
)

set ECH=
if "%showcommand%" == "1" (
  set ECH=echo
)
if "x%hostname%" == "x" (
  echo "***error: host not specified.  Enter a host using -host "
  exit /b
)

set command=plink %plink_options% %username%@%hostname% runsmv_ssh.sh %exe% %casedir% %casename% %renderdir%
%ECH% %command%

goto eof

:getopts
 if (%1)==() exit /b
 set valid=0
 set arg=%1
 set firstchar=%arg:~0,1%
 set casename=%1
 
 if /I "%1" EQU "-host" (
   set valid=1
   set hostname=%2
   shift
 )
 if /I "%1" EQU "-casedir" (
   set casedir=%2
   set valid=1
   shift
 )
 if /I "%1" EQU "-renderdir" (
   set renderdir=%2
   set valid=1
   shift
 )
 if /I "%1" EQU "-exe" (
   set exe=%2
   set valid=1
   shift
 )
 if /I "%1" EQU "-help" (
   call :usage
   set stopscript=1
   exit /b
 )
 if /I "%1" EQU "-h" (
   call :usage
   set stopscript=1
   exit /b
 )
 if /I "%1"  EQU "-v" (
   set showcommand=1
   set valid=1
 )

 shift
 if %valid% == 0 (
   if %firstchar% == "-" (
     echo.
     echo ***Error: the input argument %arg% is invalid
     echo.
     echo Usage:
     call :usage
     set stopscript=1
     exit /b
   )
 )
if not (%1)==() goto getopts
exit /b

:usage  
echo smv2html [options]
echo. 
echo -help                - display this message
echo -host hostname       - computer where smokeview will be run
if "x%hostname%" == "x" goto skip_hostname
echo                        (default: %hostname%)
:skip_hostname
echo -casedir directory   - directory where case is located
echo                        (default: %casedir%)
echo -renderdir directory - directory where html files are placed
echo                        (default: %renderdir%)
echo -exe executable      - smokeview used to convert data
echo                        (default: smokeview)
echo -v                   - show command that will be run
exit /b

:normalise
set temparg=%~f1
exit /b

:eof

