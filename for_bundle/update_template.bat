@echo off

set CURDIR=%CD%
cd ..
set ROOT=%CD%
cd %CURDIR%
set MAKEPODIR=%ROOT%\Build\makepo\intel_win_64
set MAKEPO=%MAKEPODIR%\makepo_win_64
set SMVDIR=%ROOT%\Source%\smokeview
set SHAREDDIR=%ROOT%\Source%\shared
set OUT=smokeview_template.pl
set sort=%userprofile%\bin\sort


::if [ ! -e $MAKEPO ]; then
::  echo "***warning: The application $MAKEPO does not exist."
::  echo "Building makepo"
::  cd $MAKEPODIR
::  ./make_makepo.sh
::  if [ ! -e $MAKEPO ]; then
::    echo "***error: The application $MAKEPO failed to build. Script aborted."
::    cd $CURDIR
::    exit
::  fi
::  cd $CURDIR
::fi

echo updating smokeview_template.po

cat %SHAREDDIR%\*.h %SHAREDDIR%\*.c %SMVDIR%\*.h %SMVDIR%\*.c %SMVDIR%\*.cpp | %MAKEPO% | %sort% -u | %MAKEPO% -a    > %OUT%
