@echo off
set OPTS=i
set arg1=%1
set arg2=%2
:: if arg3 is freeglut then freeglut not glut library will be built
set arg3=%3

:: setup compiler environment
if x%arg1% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

set EXIT_SCRIPT=1

set WAIT=
if "%arg1%"=="bot" (
  set WAIT=/WAIT
)

set LIBDIR=%CD%
git clean -dxf

cd ..\..\..\Source
set SRCDIR=%CD%

cd ..\Build
set BUILDDIR=%CD%

:: ZLIB
cd %SRCDIR%\zlib131
start "building windows zlib" %WAIT% cmd /c "makelib %OPTS% -copy libz.lib     %LIBDIR%\zlib.lib             > %LIBDIR%\zlib.out 2>&1 "
      
:: JPEG
cd %SRCDIR%\jpeg-9b
start "building windows jpeg" %WAIT% cmd /c "makelib %OPTS% -copy libjpeg.lib  %LIBDIR%\jpeg.lib             > %LIBDIR%\jpeg.out 2>&1 "

:: PNG
cd %SRCDIR%\png-1.6.48
start "" cmd /c "makelib %OPTS% -copy libpng.lib   %LIBDIR%\png.lib              > %LIBDIR%\png.out 2>&1 "

:: GD
cd %SRCDIR%\gd-2.3.3
start "building windows gd" %WAIT% cmd /c "makelib %OPTS% -copy libgd.lib %LIBDIR%\gd.lib              > %LIBDIR%\gd.out 2>&1 "

:: GLUT
if x%arg3% == xfreeglut goto skip_glut
  cd %SRCDIR%\glut-3.7.6
start "building windows glut" %WAIT% cmd /c "makelib %OPTS% -copy libglutwin.lib %LIBDIR%\glut32.lib   > %LIBDIR%\glut32.out 2>&1 "
:skip_glut

:: GLUI
cd %SRCDIR%\glui_v2_1_beta
if x%arg3% == xfreeglut goto skip_glui1
  start "building windows glui" %WAIT% cmd /c "makelib %OPTS% -copy libglui.lib %LIBDIR%\glui.lib         > %LIBDIR%\glui.out 2>&1 "
:skip_glui1

if NOT x%arg3% == xfreeglut goto skip_glui2
  start "building windows glui" %WAIT% cmd /c "makelib_freeglut %OPTS% -copy libglui.lib %LIBDIR%\glui.lib > %LIBDIR%\glui.out 2>&1 "
:skip_glui2

:: pthreads
cd %SRCDIR%\pthreads
start "building windows pthreads" %WAIT% cmd /c "makelib %OPTS% -copy libpthreads.lib %LIBDIR%\pthreads.lib > %LIBDIR%\pthread.out 2>&1 "

:: FREEGLUT
if NOT x%arg3% == xfreeglut goto skip_freeglut
cd %BUILDDIR%\freeglut3.0.0\gnu_win
call make_freeglut %OPTS%
copy freeglut_staticd.lib %LIBDIR%\freeglut_staticd.lib
copy freeglut_staticd.lib %LIBDIR%\glut32.lib
:skip_freeglut

cd %LIBDIR%

if "x%arg1%" == "xbot" goto skip1
pause
dir *.lib
pause
:skip1
