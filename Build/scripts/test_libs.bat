@echo off
setlocal
set LIBDIR=%1
set CURDIR=%CD%

cd %LIBDIR%
if NOT exist %LIBDIR%\gd.lib       goto BUILDLIBS
if NOT exist %LIBDIR%\glui.lib     goto BUILDLIBS
::if NOT exist %LIBDIR%\glut32.lib   goto BUILDLIBS
if NOT exist %LIBDIR%\jpeg.lib     goto BUILDLIBS
if NOT exist %LIBDIR%\png.lib      goto BUILDLIBS
if NOT exist %LIBDIR%\pthreads.lib goto BUILDLIBS
if NOT exist %LIBDIR%\zlib.lib     goto BUILDLIBS
cd %CURDIR%
exit /b

:BUILDLIBS
call make_LIBS %GLUT%
cd %CURDIR%