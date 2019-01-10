@echo off
set OPTS=g

set LIBDIR=%CD%
set SRCDIR=%LIBDIR%\..\..\..\Source
erase *.a

:: ZLIB
cd %SRCDIR%\zlib128
call makelib %OPTS% 
copy libz.a %LIBDIR%\zlib.a

:: JPEG
cd %SRCDIR%\jpeg-9b
call makelib %OPTS% 
copy libjpeg.a  %LIBDIR%\jpeg.a

:: PNG
cd %SRCDIR%\png-1.6.21
call makelib %OPTS% 
copy libpng.a %LIBDIR%\png.a

:: GD
cd %SRCDIR%\gd-2.0.15
call makelib %OPTS% 
copy libgd.a %LIBDIR%\gd.a

:: GLUT
cd %SRCDIR%\glut-3.7.6
call makelib %OPTS% 
copy libglutwin.a %LIBDIR%\glut32.a

:: GLUI
cd %SRCDIR%\glui_v2_1_beta
call makelib %OPTS% 
copy libglui.a %LIBDIR%\glui.a

:: pthreads
cd %SRCDIR%\pthreads
call makelib %OPTS% 
copy libpthreads.a %LIBDIR%\pthreads.a

cd %LIBDIR%

echo library builds complete
