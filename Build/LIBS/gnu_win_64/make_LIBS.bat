@echo off
set OPTS=g
set arg1=%1
set arg2=%2

set WAIT=
if "%arg1%"=="bot" (
  set WAIT=/WAIT
)

set LIBDIR=%CD%
set SRCDIR=%LIBDIR%\..\..\..\Source
erase *.lib

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
cd %SRCDIR%\glut-3.7.6
start %WAIT% makelib %OPTS% -copy libglutwin.a %LIBDIR%\glut32.a

:: GLUI
cd %SRCDIR%\glui_v2_1_beta
start %WAIT% makelib %OPTS% -copy libglui.a %LIBDIR%\glui.a

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

cd %LIBDIR%

echo library builds complete
if x%arg1% == xbot goto skip1
pause
:skip1
