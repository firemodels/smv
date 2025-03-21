@echo off
setlocal

set GH_OWNER=firemodels
set GH_REPO=test_bundles

call :getopts %*
if %stopscript% == 1 (
  exit /b 0
)

:: get FDS figures if both FDS and SMV and not set
if "x%FDS%" == "x" if "x%SMV%" == "x" set FDS=1

:: get smokeview user guide figures if guides vars are not set
if "x%SMV%" == "x" goto endif1
  if "x%USER%" == "x" if "x%VER%" == "x" set USER=1
:endif1

:: get fds user guide figures if guide vars are not set
if "x%FDS%" == "x" goto endif2
  if "x%USER%" == "x" if "x%VER%" == "x" if "x%TECH%" == "x" if "x%VAL%" == "x" set USER=1
:endif2

set CURDIR=%CD%
cd files
set FILESDIR=%CD%

echo ***cleaning %FILESDIR%
git clean -dxf

cd %CURDIR%

cd ..\..\..
set ROOT=%CD%


cd %ROOT%\smv
set SMVREPO=%CD%

cd %ROOT%\fds
set FDSREPO=%CD%

cd %CURDIR%

if "x%SMV%" == "x" goto endif_smv
  if "x%USER%" == "x" goto endif_smv_ug
    call :DOWNLOADFILE SMOKEVIEW_TEST SMV_UG_figures.tar.gz
    call :COPYFILES %SMVREPO%\Manuals\SMV_User_Guide\SCRIPT_FIGURES       SMV_UG_figures.tar.gz
  :endif_smv_ug

  if "x%VER%" == "x" goto endif_smv_verg
    call :DOWNLOADFILE SMOKEVIEW_TEST SMV_VG_figures.tar.gz
    call :COPYFILES %SMVREPO%/Manuals/SMV_Verification_Guide/SCRIPT_FIGURES SMV_VG_figures.tar.gz
  :endif_smv_verg
:endif_smv

if "x%FDS%" == "x" goto endif_fds
  if "x%USER%" == "x" goto endif_fds_ug
    call :DOWNLOADFILE FDS_TEST FDS_UG_figures.tar.gz
    call :COPYFILES %FDSREPO%\Manuals\FDS_User_Guide\SCRIPT_FIGURES               FDS_UG_figures.tar.gz
  :endif_fds_ug

  if "x%TECH%" == "x" goto endif_fds_techg
    call :DOWNLOADFILE FDS_TEST FDS_TG_figures.tar.gz
    call :COPYFILES %FDSREPO%\Manuals\FDS_Technical_Reference_Guide\SCRIPT_FIGURES FDS_TG_figures.tar.gz
  :endif_fds_techg

  if "x%VER%" == "x" goto endif_fds_verg
    call :DOWNLOADFILE FDS_TEST FDS_VERG_figures.tar.gz
    call :COPYFILES %FDSREPO%\Manuals\FDS_Verification_Guide\SCRIPT_FIGURES        FDS_VERG_figures.tar.gz
  :endif_fds_verg

  if "x%VAL%" == "x" goto endif_fds_valg
    call :DOWNLOADFILE FDS_TEST FDS_VALG_figures.tar.gz
    call :COPYFILES %FDSREPO%\Manuals\FDS_Validation_Guide\SCRIPT_FIGURES          FDS_VALG_figures.tar.gz
  :endif_fds_valg
:endif_fds

goto eof

::-----------------------------------------------------------------------
:DOWNLOADFILE
::-----------------------------------------------------------------------
  set TAG=%1
  set FILE=%2
  echo.
  echo downloading %FILE% to %FILESDIR% using:
  echo gh release download %TAG% -p %FILE% -D %FILESDIR%  -R github.com/%GH_OWNER%/%GH_REPO%
  gh release download %TAG% -p %FILE% -D %FILESDIR%  -R github.com/%GH_OWNER%/%GH_REPO%
  exit /b

::-----------------------------------------------------------------------
:COPYFILES
::-----------------------------------------------------------------------
  set TODIR=%1
  set FILE=%2
  if NOT EXIST %TODIR% goto copy_else1
    echo untarring %FILE% to %TODIR%
    cd %TODIR%
    if NOT EXIST %FILESDIR%\%FILE% goto copy_else2
      tar xf %FILESDIR%\%FILE% 
      git checkout .gitignore 2> Nul
      goto copy_endif1
    :copy_else2
      echo "***error: %FILESDIR%\%FILE% does not exist"
      goto copy_endif1
    copy_endif2
  :copy_else1
    echo ***error: %TODIR% does not exist
  :copy_endif1
  exit /b


::-----------------------------------------------------------------------
:usage
::-----------------------------------------------------------------------

echo Usage:
echo GetFigures options
echo.
echo Download FDS and/or Smokeview manual figures from github and copy to their
echo respecitve manual SCRIPT_DIRS directories
echo.
echo Options:
echo -a - copy all FDS and Smokeview manual figures
echo -f - copy FDS figures for manuals specified by -u, -v, -V and or -t options
echo -F - copy FDS figures for all guides
echo -h - display this message
echo -f - copy Smokeview figures for manuals specified by -u and/or -v options
echo -S - copy Smokeview figures for both user and verification guides
echo -t - copy FDS technical guide figures
echo -u - copy user guide figures  (also specify either -f or -s)
echo -v - copy verification guide figures  (also specify either -f or -s)
echo -V - copy FDS validation guide figures
exit /b 0

::-----------------------------------------------------------------------
:getopts
::-----------------------------------------------------------------------
 set stopscript=0
 if (%1)==() exit /b
 set valid=0
 set arg=%1
 if /I "%1" EQU "-h" (
   call :usage
   set stopscript=1
   exit /b
 )
 if /I "%1" EQU "-a" (
   set valid=1
   set FDS=1
   set SMV=1
   set TECH=1
   set USER=1
   set VAL=1
   set VER=1
 )
 if "%1" EQU "-f" (
   set valid=1
   set FDS=1
 )
 if "%1" EQU "-F" (
   set valid=1
   set FDS=1
   set TECH=1
   set USER=1
   set VAL=1
   set VER=1
 )
 if "%1" EQU "-s" (
   set valid=1
   set SMV=1
 )
 if "%1" EQU "-S" (
   set valid=1
   set SMV=1
   set USER=1
   set VER=1
 )
 if /I "%1" EQU "-t" (
   set valid=1
   set TECH=1
 )
 if /I "%1" EQU "-u" (
   set valid=1
   set USER=1
 )
 if "%1" EQU "-v" (
   set valid=1
   set VER=1
 )
 if "%1" EQU "-V" (
   set valid=1
   set VAL=1
 )
 shift
 if %valid% == 0 (
   echo.
   echo ***Error: the input argument %arg% is invalid
   echo.
   echo Usage:
   call :usage
   set stopscript=1
   exit /b 1
 )
if not (%1)==() goto getopts
exit /b 0


if [[ "$FDS" == "" ]] && [[ "$SMV" == "" ]]; then
  FDS=1
fi

if [ "SMV" != "" ]; then
  if [ "$USER" == "" ]] && [[ "$VER" == "" ]]; then
    USER=1
  fi
fi

if [ "FDS" != "" ]; then
  if [[ "$USER" == "" ]] && [[ "$VER" == "" ]] && [[ "$VAL" == "" ]] && [[ "$TECH" == "" ]]; then
    USER=1
  fi
fi

:eof
