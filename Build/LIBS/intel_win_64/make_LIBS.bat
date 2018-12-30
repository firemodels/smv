@echo off
set OPTS=i
set arg1=%1
set arg2=%2
:: if arg3 is freeglut then freeglut not glut library will be built
set arg3=%3

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
cd %SRCDIR%\zlib128
start %WAIT% makelib %OPTS% -copy libz.lib %LIBDIR%\zlib.lib

:: JPEG
cd %SRCDIR%\jpeg-9b
start %WAIT% makelib %OPTS% -copy libjpeg.lib  %LIBDIR%\jpeg.lib

:: PNG
cd %SRCDIR%\png-1.6.21
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

if NOT x%arg2% == xlua goto skip_lua
:: Lua interpreter
cd %SRCDIR%\lua-5.3.1
call makelib.bat
copy src\liblua.lib %LIBDIR%\liblua.lib

:: LPEG
cd %SRCDIR%\lpeg-1.0.0
call makelib.bat
copy lpeg.lib %LIBDIR%\lpeg.lib
:skip_lua

:: FREEGLUT
if NOT x%arg3% == xfreeglut goto skip_freeglut
cd %BUILDDIR%\freeglut3.0.0\intel_win_64
call make_freeglut %OPTS% 
copy freeglut_staticd.lib %LIBDIR%\freeglut_staticd.lib
copy freeglut_staticd.lib %LIBDIR%\glut32.lib
:skip_freeglut

cd %LIBDIR%

echo library builds complete
if x%arg1% == xbot goto skip1
pause
:skip1
