@echo off
set OPTS=c
set glutopt=%1

set LIBDIR=%CD%

cd %LIBDIR%\..\..\..\Source
set SRCDIR=%CD%

cd %LIBDIR%\..\..\..\Build
set BUILDDIR=%CD%

erase *.lib

:: ZLIB
cd %SRCDIR%\zlib131
call makelib %OPTS% 
copy libz.lib %LIBDIR%\zlib.lib

:: JPEG
cd %SRCDIR%\jpeg-9b
call makelib %OPTS% 
copy libjpeg.lib  %LIBDIR%\jpeg.lib

:: PNG
cd %SRCDIR%\png-1.6.48
call makelib %OPTS% 
copy libpng.lib %LIBDIR%\png.lib

:: GD
cd %SRCDIR%\gd-2.3.3
call makelib %OPTS% 
copy libgd.lib %LIBDIR%\gd.lib

:: GLUT
if x%glutopt% == xfreeglut goto skip_glut
cd %SRCDIR%\glut-3.7.6
call makelib %OPTS% 
copy libglutwin.lib %LIBDIR%\glut32.lib
:skip_glut

:: FREEGLUT
if NOT x%glutopt% == xfreeglut goto skip_freeglut
cd %BUILDDIR%\freeglut3.0.0\gnu_win_64
call make_freeglut %OPTS% 
copy freeglut_staticd.lib %LIBDIR%\freeglut_staticd.lib
:skip_freeglut

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
