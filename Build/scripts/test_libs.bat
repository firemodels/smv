@echo off
setlocal
set LIBDIR=%1

set CURDIR=%CD%
cd %LIBDIR%
set LIBDIR=%CD%
cd %CURDIR%

if NOT exist %LIBDIR%\gd.lib       goto BUILDLIBS
if NOT exist %LIBDIR%\glui.lib     goto BUILDLIBS
if NOT exist %LIBDIR%\glut32.lib   goto BUILDLIBS
if NOT exist %LIBDIR%\jpeg.lib     goto BUILDLIBS
if NOT exist %LIBDIR%\png.lib      goto BUILDLIBS
if NOT exist %LIBDIR%\pthreads.lib goto BUILDLIBS
if NOT exist %LIBDIR%\zlib.lib     goto BUILDLIBS
goto eof

:BUILDLIBS
cd %LIBDIR%
call make_LIBS 
cd %CURDIR%

:eof
