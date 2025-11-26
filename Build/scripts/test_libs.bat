@echo off
setlocal
set LIBDIR=%1

set BUILDLIBS=0
set CURDIR=%CD%
cd %LIBDIR%
set LIBDIR=%CD%
cd %CURDIR%

if NOT exist %LIBDIR%\intel_win\gd.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\intel_win\glui.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\intel_win\glut32.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\intel_win\jpeg.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\intel_win\png.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\intel_win\pthreads.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\intel_win\zlib.lib set BUILDLIBS=1

if %BUILDLIBS% == 0 goto eof

cd %LIBDIR%\intel_win
call make_LIBS_bot 
cd %CURDIR%

:eof
