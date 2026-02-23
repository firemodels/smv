@echo off
set OPTS=%1
set bot=%2
set freegluttype=%3

:: setup compiler environment
if NOT %COMPILER% == icx goto skip1
if not defined ONEAPI_ROOT call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat > Nul
if defined ONEAPI_ROOT echo *** environment for icx defined
if not defined ONEAPI_ROOT echo ***error: setup for icx failed
if not defined ONEAPI_ROOT exit /b
:skip1

set LIBDIR=%CD%
echo *** Cleaning %CD%
git clean -dxf > Nul

cd ..\..\..\Source
set SRCDIR=%CD%

cd ..\Build
set BUILDDIR=%CD%

cd %LIBDIR%\..
set COMMON=%CD%

:: ZLIB
cd %SRCDIR%\zlib131
echo *** building zlib
start "building zlib"  cmd /c  "%COMMON%\lib_wrapper zlib %LIBDIR% makelib %OPTS%  > %LIBDIR%\zlib.out 2>&1"
      
:: JPEG
cd %SRCDIR%\jpeg-9b
echo *** building jpeg
start "building jpeg"  cmd /c  "%COMMON%\lib_wrapper jpeg %LIBDIR% makelib %OPTS%  > %LIBDIR%\jpeg.out 2>&1"

:: PNG
cd %SRCDIR%\png-1.6.48
echo *** building png
start "building png"   cmd /c  "%COMMON%\lib_wrapper png  %LIBDIR% makelib %OPTS%  > %LIBDIR%\png.out 2>&1"

:: GD
cd %SRCDIR%\gd-2.3.3
echo *** building gd
start "building gd"    cmd /c  "%COMMON%\lib_wrapper gd   %LIBDIR% makelib %OPTS%  > %LIBDIR%\gd.out 2>&1"

:: pthreads
cd %SRCDIR%\pthreads
echo *** building pthreads
start "building pthreads"  cmd /c "%COMMON%\lib_wrapper pthreads %LIBDIR% makelib %OPTS%  > %LIBDIR%\pthreads.out 2>&1"

:: GLUT%
if NOT x%freegluttype% == x goto skip_glut
  cd %SRCDIR%\glut-3.7.6
  echo *** building glut
  start "building glut" /WAIT cmd /c "makelib %OPTS%  > %LIBDIR%\glut.out 2>&1"
  echo *** glut built
:skip_glut

:: freeglut
if x%freegluttype% == x goto skip_freeglut
  cd %BUILDDIR%\freeglut\%freegluttype%
  echo *** building freeglut
  start "building freeglut" /WAIT cmd /c "makelib %OPTS%  > %LIBDIR%\freeglut.out 2>&1"
  echo *** freeglut built
:skip_freeglut

:: GLUI
cd %SRCDIR%\glui_v2_1_beta
echo *** building glui
start "building glui"  cmd /c "%COMMON%\lib_wrapper glui %LIBDIR% makelib %OPTS%  > %LIBDIR%\glui.out 2>&1"

call :WAIT zlib.running
echo *** zlib built

call :WAIT jpeg.running
echo *** jpeg built

call :WAIT png.running
echo *** png built

call :WAIT gd.running
echo *** gd built

call :WAIT pthreads.running
echo *** pthreads built

call :WAIT glui.running
echo *** glui built


cd %LIBDIR%

call :COPY %SRCDIR%\zlib131\libz.lib          %LIBDIR%\zlib.lib
call :COPY %SRCDIR%\jpeg-9b\libjpeg.lib       %LIBDIR%\jpeg.lib
call :COPY %SRCDIR%\png-1.6.48\libpng.lib     %LIBDIR%\png.lib
call :COPY %SRCDIR%\gd-2.3.3\libgd.lib        %LIBDIR%\gd.lib

if NOT x%freeglutype% == x goto skip_glut2
call :COPY %SRCDIR%\glut-3.7.6\libglutwin.lib %LIBDIR%\glut32.lib
:skip_glut2

call :COPY %SRCDIR%\glui_v2_1_beta\libglui.lib %LIBDIR%\glui.lib

if x%freegluttype% == x goto skip_freeglut
call :COPY %BUILDDIR%\freeglut\lib\libglut.lib %LIBDIR%\freeglut.lib
:skip_freeglut

call :COPY %SRCDIR%\pthreads\libpthreads.lib   %LIBDIR%\pthreads.lib

goto eof

::------------------------------------------------
:COPY
::------------------------------------------------
set label=%~n1%~x1
set infile=%1
set outfile=%2
IF NOT EXIST %infile% goto else1
   echo *** copying %label%
   copy %infile% %outfile% >Nul
   goto endif1
:else1
   echo.
   echo *** warning: %infile% does not exist
   echo.
:endif1
exit /b

::------------------------------------------------
:WAIT
::------------------------------------------------
set file=%1
if not exist %LIBDIR%\%file% exit /b
timeout /t 1 > Nul
goto WAIT

:eof
