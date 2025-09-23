@echo off
set OPTS=c

:: arg1: bot
set arg1=%1

:: arg2: lua
set arg2=%2

:: arg3: glut/freeglut
set arg3=%3

set EXIT_SCRIPT=1

set WAIT=
if "%arg1%"=="bot" (
  set WAIT=/WAIT
)

set LIBDIR=%CD%
erase *.lib

cd ..\..\..\Source
set SRCDIR=%CD%

cd ..\Build
set BUILDDIR=%CD%

:: ZLIB
cd %SRCDIR%\zlib131
start %WAIT% makelib %OPTS% -copy libz.lib %LIBDIR%\zlib.lib

:: JPEG
cd %SRCDIR%\jpeg-9b
start %WAIT% makelib %OPTS% -copy libjpeg.lib  %LIBDIR%\jpeg.lib

:: PNG
cd %SRCDIR%\png-1.6.48
start %WAIT% makelib %OPTS% -copy libpng.lib %LIBDIR%\png.lib

:: GD
cd %SRCDIR%\gd-2.0.15
start %WAIT% call makelib %OPTS% -copy libgd.lib %LIBDIR%\gd.lib

:: GLUT
if x%arg3% == xfreeglut goto skip_glut
  cd %SRCDIR%\glut-3.7.6
  start %WAIT% makelib %OPTS% -copy libglutwin.lib %LIBDIR%\glut32.lib
:skip_glut

:: GLUI
cd %SRCDIR%\glui_v2_1_beta
if x%arg3% == xfreeglut goto skip_glui1
  start %WAIT% makelib %OPTS% -copy libglui.lib %LIBDIR%\glui.lib
:skip_glui1

if NOT x%arg3% == xfreeglut goto skip_glui2
  start %WAIT% makelib_freeglut %OPTS% -copy libglui.lib %LIBDIR%\glui.lib
:skip_glui2

:: pthreads
cd %SRCDIR%\pthreads
start %WAIT% makelib %OPTS% -copy libpthreads.lib %LIBDIR%\pthreads.lib

:: FREEGLUT
if NOT x%arg3% == xfreeglut goto skip_freeglut
cd %BUILDDIR%\freeglut3.0.0\gnu_win_64
call make_freeglut %OPTS%
copy freeglut_staticd.lib %LIBDIR%\freeglut_staticd.lib
:skip_freeglut

cd %LIBDIR%

echo library builds complete
if x%arg1% == xbot goto skip1
pause
:skip1
