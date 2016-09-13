@echo off
set LIBDIR=%1

set CURDIR=%CD%
cd %LIBDIR%
set LIBDIR=%CD%
cd %CURDIR%

call :makelibs libgd.lib
call :makelibs libglui.lib 
call :makelibs libglut.lib 
call :makelibs libjpeg.lib 
call :makelibs libpng.lib
call :makelibs libz.lib
goto eof

:makelibs
  set lib=%1
  if NOT exist %LIBDIR%\intel_win_64\%lib% (
    set CURDIR=%CD%`
    cd %LIBDIR%\intel_win_64
    call makelibs 
    cd %CURDIR%
  )
  exit /b

:eof
