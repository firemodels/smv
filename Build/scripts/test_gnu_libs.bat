@echo off
setlocal
set LIBDIR=%1
set GLUT=%2

set BUILDLIBS=0
set CURDIR=%CD%
cd %LIBDIR%
set LIBDIR=%CD%
cd %CURDIR%

if NOT exist %LIBDIR%\gnu_win\gd.a       set BUILDLIBS=1
if NOT exist %LIBDIR%\gnu_win\glui.a     set BUILDLIBS=1

if NOT x%GLUT% == xglut goto skip_glut
  if NOT exist %LIBDIR%\gnu_win\glut32.a set BUILDLIBS=1
:skip_glut

if NOT x%GLUT% == xfreeglut goto skip_freeglut
  if NOT exist %LIBDIR%\gnu_win\freeglut_staticd.a   set BUILDLIBS=1
:skip_freeglut

if NOT exist %LIBDIR%\gnu_win\jpeg.a     set BUILDLIBS=1
if NOT exist %LIBDIR%\gnu_win\png.a      set BUILDLIBS=1
if NOT exist %LIBDIR%\gnu_win\pthreads.a set BUILDLIBS=1
if NOT exist %LIBDIR%\gnu_win\zlib.a     set BUILDLIBS=1

if %BUILDLIBS% == 0 goto eof

cd %LIBDIR%\gnu_win
call make_LIBS_bot %GLUT%
cd %CURDIR%

:eof
