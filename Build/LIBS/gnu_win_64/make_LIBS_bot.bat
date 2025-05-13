@echo off
set OPTS=g
set glutopt=%1

set LIBDIR=%CD%

cd %LIBDIR%\..\..\..\Source
set SRCDIR=%CD%

cd %LIBDIR%\..\..\..\Build
set BUILDDIR=%CD%

erase *.a

:: ZLIB
cd %SRCDIR%\zlib131
call makelib %OPTS% 
copy libz.a %LIBDIR%\zlib.a

:: JPEG
cd %SRCDIR%\jpeg-9b
call makelib %OPTS% 
copy libjpeg.a  %LIBDIR%\jpeg.a

:: PNG
cd %SRCDIR%\png-1.6.48
call makelib %OPTS% 
copy libpng.a %LIBDIR%\png.a

:: GD
cd %SRCDIR%\gd-2.0.15
call makelib %OPTS% 
copy libgd.a %LIBDIR%\gd.a

:: GLUT
if x%glutopt% == xfreeglut goto skip_glut
cd %SRCDIR%\glut-3.7.6
call makelib %OPTS% 
copy libglutwin.a %LIBDIR%\glut32.a
:skip_glut

:: FREEGLUT
if NOT x%glutopt% == xfreeglut goto skip_freeglut
cd %BUILDDIR%\freeglut3.0.0\gnu_win_64
call make_freeglut %OPTS% 
copy freeglut_staticd.a %LIBDIR%\freeglut_staticd.a
:skip_freeglut

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
