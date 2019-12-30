@echo off
setlocal
set LIBDIR=%1
set GLUT=%2

set BUILDLIBS=0
set CURDIR=%CD%
cd %LIBDIR%
set LIBDIR=%CD%
cd %CURDIR%

if NOT exist %LIBDIR%\gnu_win_64\gd.a       set BUILDLIBS=1
if NOT exist %LIBDIR%\gnu_win_64\glui.a     set BUILDLIBS=1

if NOT x%GLUT% == xglut goto skip_glut
  if NOT exist %LIBDIR%\gnu_win_64\glut32.a set BUILDLIBS=1
:skip_glut

if NOT x%GLUT% == xfreeglut goto skip_freeglut
  if NOT exist %LIBDIR%\gnu_win_64\freeglut_staticd.a   set BUILDLIBS=1
:skip_freeglut

if NOT exist %LIBDIR%\gnu_win_64\jpeg.a     set BUILDLIBS=1
if NOT exist %LIBDIR%\gnu_win_64\png.a      set BUILDLIBS=1
if NOT exist %LIBDIR%\gnu_win_64\pthreads.a set BUILDLIBS=1
if NOT exist %LIBDIR%\gnu_win_64\zlib.a     set BUILDLIBS=1

if %BUILDLIBS% == 0 goto eof

cd %LIBDIR%\gnu_win_64
call make_LIBS_bot %GLUT%
cd %CURDIR%

:eof
