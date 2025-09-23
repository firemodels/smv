@echo off
setlocal
set LIBDIR=%1

set BUILDLIBS=0
set CURDIR=%CD%
cd %LIBDIR%
set LIBDIR=%CD%
cd %CURDIR%

if NOT exist %LIBDIR%\clang_win_64\gd.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\clang_win_64\glui.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\clang_win_64\glut32.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\clang_win_64\jpeg.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\clang_win_64\png.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\clang_win_64\pthreads.lib set BUILDLIBS=1
if NOT exist %LIBDIR%\clang_win_64\zlib.lib set BUILDLIBS=1

if %BUILDLIBS% == 0 goto eof

cd %LIBDIR%\clang_win_64
call make_LIBS_bot 
cd %CURDIR%

:eof
