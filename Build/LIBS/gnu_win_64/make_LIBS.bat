@echo off
set OPTS=g
set arg1=%1
set arg2=%2
set arg3=%3

set WAIT=
if "%arg1%"=="bot" (
  set WAIT=/WAIT
)

set LIBDIR=%CD%
erase *.a

cd ..\..\..\Source
set SRCDIR=%CD%

cd ..\Build
set BUILDDIR=%CD%

:: ZLIB
cd %SRCDIR%\zlib128
start %WAIT% makelib %OPTS% -copy libz.a %LIBDIR%\zlib.a

:: JPEG
cd %SRCDIR%\jpeg-9b
start %WAIT% makelib %OPTS% -copy libjpeg.a  %LIBDIR%\jpeg.a

:: PNG
cd %SRCDIR%\png-1.6.21
start %WAIT% makelib %OPTS% -copy libpng.a %LIBDIR%\png.a

:: GD
cd %SRCDIR%\gd-2.0.15
start %WAIT% call makelib %OPTS% -copy libgd.a %LIBDIR%\gd.a

:: GLUT
if x%arg3% == xfreeglut goto skip_glut
  cd %SRCDIR%\glut-3.7.6
  start %WAIT% makelib %OPTS% -copy libglutwin.a %LIBDIR%\glut32.a
:skip_glut

:: GLUI
cd %SRCDIR%\glui_v2_1_beta
if x%arg3% == xfreeglut goto skip_glui1
  start %WAIT% makelib %OPTS% -copy libglui.a %LIBDIR%\glui.a
:skip_glui1

if NOT x%arg3% == xfreeglut goto skip_glui2
  start %WAIT% makelib_freeglut %OPTS% -copy libglui.a %LIBDIR%\glui.a
:skip_glui2


:: pthreads
cd %SRCDIR%\pthreads
start %WAIT% makelib %OPTS% -copy libpthreads.a %LIBDIR%\pthreads.a

if NOT x%arg2% == xlua goto skip_lua
:: Lua interpreter
cd %SRCDIR%\lua-5.3.1
call makelib.bat
copy src\liblua.a %LIBDIR%\liblua.a

:: LPEG
cd %SRCDIR%\lpeg-1.0.0
call makelib.bat
copy lpeg.a %LIBDIR%\lpeg.a
:skip_lua

:: FREEGLUT
if NOT x%arg3% == xfreeglut goto skip_freeglut
cd %BUILDDIR%\freeglut3.0.0\gnu_win_64
call make_freeglut %OPTS% 
copy freeglut_staticd.a %LIBDIR%\freeglut_staticd.a
:skip_freeglut

cd %LIBDIR%

echo library builds complete
if x%arg1% == xbot goto skip1
pause
:skip1
