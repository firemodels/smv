@echo off
set OPTS=i

set LIBDIR=%CD%
erase *.lib

cd ..\..\..\Source
set SRCDIR=%CD%

cd ..\Build
set BUILDDIR=%CD%

:: ZLIB
cd %SRCDIR%\zlib128
call makelib %OPTS% 
copy libz.lib %LIBDIR%\zlib.lib

:: JPEG
cd %SRCDIR%\jpeg-9b
call makelib %OPTS% 
copy libjpeg.lib  %LIBDIR%\jpeg.lib

:: PNG
cd %SRCDIR%\png-1.6.21
call makelib %OPTS% 
copy libpng.lib %LIBDIR%\png.lib

:: GD
cd %SRCDIR%\gd-2.0.15
call makelib %OPTS% 
copy libgd.lib %LIBDIR%\gd.lib

:: GLUT
cd %SRCDIR%\glut-3.7.6
call makelib %OPTS% 
copy libglutwin.lib %LIBDIR%\glut32.lib

:: FREEGLUT
cd %BUILDDIR%\freeglut3.0.0\intel_win_64
call make_freeglut %OPTS% 
copy libglutwin.lib %LIBDIR%\freeglut32.lib

:: GLUI
cd %SRCDIR%\glui_v2_1_beta
call makelib %OPTS% 
copy libglui.lib %LIBDIR%\glui.lib

:: pthreads
cd %SRCDIR%\pthreads
call makelib %OPTS% 
copy libpthreads.lib %LIBDIR%\pthreads.lib

cd %LIBDIR%

echo library builds complete
