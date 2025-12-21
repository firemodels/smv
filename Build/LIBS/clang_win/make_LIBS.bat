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
start "" cmd /c "makelib %OPTS% -copy libz.lib     %LIBDIR%\zlib.lib             > %LIBDIR%\zlib.out 2>&1 "
      
:: JPEG
cd %SRCDIR%\jpeg-9b
start "" cmd /c "makelib %OPTS% -copy libjpeg.lib  %LIBDIR%\jpeg.lib             > %LIBDIR%\jpeg.out 2>&1 "

:: PNG
cd %SRCDIR%\png-1.6.48
start "" cmd /c "makelib %OPTS% -copy libpng.lib   %LIBDIR%\png.lib              > %LIBDIR%\png.out 2>&1 "

:: GD
cd %SRCDIR%\gd-2.3.3
start "" cmd /c "makelib %OPTS% -copy libgd.lib %LIBDIR%\gd.lib              > %LIBDIR%\gd.out 2>&1 "

:: GLUT
if x%arg3% == xfreeglut goto skip_glut
  cd %SRCDIR%\glut-3.7.6
  start "" cmd /c "makelib %OPTS% -copy libglutwin.lib %LIBDIR%\glut32.lib   > %LIBDIR%\glut32.out 2>&1 "
:skip_glut

:: GLUI
cd %SRCDIR%\glui_v2_1_beta
if x%arg3% == xfreeglut goto skip_glui1
  start "" cmd /c "makelib %OPTS% -copy libglui.lib %LIBDIR%\glui.lib         > %LIBDIR%\glui.out 2>&1 "
:skip_glui1

if NOT x%arg3% == xfreeglut goto skip_glui2
  start "" cmd /c "makelib_freeglut %OPTS% -copy libglui.lib %LIBDIR%\glui.lib > %LIBDIR%\glui.out 2>&1 "
:skip_glui2

:: pthreads
cd %SRCDIR%\pthreads
start "" cmd /c "makelib %OPTS% -copy libpthreads.lib %LIBDIR%\pthreads.lib > %LIBDIR%\pthread.out 2>&1 "

:: FREEGLUT
if NOT x%arg3% == xfreeglut goto skip_freeglut
cd %BUILDDIR%\freeglut3.0.0\gnu_win
call make_freeglut %OPTS%
copy freeglut_staticd.lib %LIBDIR%\freeglut_staticd.lib
:skip_freeglut

cd %LIBDIR%

echo library builds complete
if x%arg1% == xbot goto skip1
pause
:skip1
