@echo off
set COMPILER_TYPE=%1
set freeglutdir=%2

set LIBDIR=%CD%
echo *** Cleaning %CD%
git clean -dxf > Nul

cd ..\..\..\Source
set SRCDIR=%CD%

cd ..\Build
set BUILDDIR=%CD%

cd ..\Utilities
set UTILDIR=%CD%

cd %LIBDIR%\..
set COMMON=%CD%

setlocal
if %COMPILER_TYPE% == i call %UTILDIR%\Scripts\setup_intel_compilers.bat
if %COMPILER_TYPE% == c call %UTILDIR%\Scripts\setup_clang_compilers.bat

:: ZLIB
cd %SRCDIR%\zlib131
echo *** building zlib
start "building zlib"  cmd /c  "%COMMON%\lib_wrapper zlib %LIBDIR% makelib %COMPILER_TYPE%  > %LIBDIR%\zlib.out 2>&1"
      
:: JPEG
cd %SRCDIR%\jpeg-9b
echo *** building jpeg
start "building jpeg"  cmd /c  "%COMMON%\lib_wrapper jpeg %LIBDIR% makelib %COMPILER_TYPE%  > %LIBDIR%\jpeg.out 2>&1"

:: PNG
cd %SRCDIR%\png-1.6.48
echo *** building png
start "building png"   cmd /c  "%COMMON%\lib_wrapper png  %LIBDIR% makelib %COMPILER_TYPE%  > %LIBDIR%\png.out 2>&1"

:: GD
cd %SRCDIR%\gd-2.3.3
echo *** building gd
start "building gd"    cmd /c  "%COMMON%\lib_wrapper gd   %LIBDIR% makelib %COMPILER_TYPE%  > %LIBDIR%\gd.out 2>&1"

:: pthreads
cd %SRCDIR%\pthreads
echo *** building pthreads
start "building pthreads"  cmd /c "%COMMON%\lib_wrapper pthreads %LIBDIR% makelib %COMPILER_TYPE%  > %LIBDIR%\pthreads.out 2>&1"

:: GLUT%
if NOT x%freeglutdir% == x goto skip_glut
  endlocal
  if %COMPILER_TYPE% == i call %UTILDIR%\Scripts\setup_intel_compilers.bat
  if %COMPILER_TYPE% == c call %UTILDIR%\Scripts\setup_clang_compilers.bat force
  cd %SRCDIR%\glut-3.7.6
  echo *** building glut
  start "building glut" /WAIT cmd /c "makelib %COMPILER_TYPE%  > %LIBDIR%\glut.out 2>&1"
  echo *** glut build complete
  timeout /t 1 > Nul
:skip_glut

:: freeglut
if x%freeglutdir% == x goto skip_freeglut
  endlocal
  if %COMPILER_TYPE% == i call %UTILDIR%\Scripts\setup_intel_compilers.bat
  if %COMPILER_TYPE% == c call %UTILDIR%\Scripts\setup_clang_compilers.bat force
  cd %BUILDDIR%\freeglut\%freeglutdir%
  echo *** building freeglut
  call make_freeglut
  echo *** freeglut build complete
:skip_freeglut

:: GLUI
cd %SRCDIR%\glui_v2_1_beta
echo *** building glui
start "building glui"  cmd /c "%COMMON%\lib_wrapper glui %LIBDIR% makelib %COMPILER_TYPE%  %freeglutdir% > %LIBDIR%\glui.out 2>&1"

call :WAIT zlib.running
echo *** zlib build complete

call :WAIT jpeg.running
echo *** jpeg build complete

call :WAIT png.running
echo *** png build complete

call :WAIT gd.running
echo *** gd build complete

call :WAIT pthreads.running
echo *** pthreads build complete

call :WAIT glui.running
echo *** glui build complete

cd %LIBDIR%

call :COPY %SRCDIR%\zlib131\libz.lib          %LIBDIR%\zlib.lib
call :COPY %SRCDIR%\jpeg-9b\libjpeg.lib       %LIBDIR%\jpeg.lib
call :COPY %SRCDIR%\png-1.6.48\libpng.lib     %LIBDIR%\png.lib
call :COPY %SRCDIR%\gd-2.3.3\libgd.lib        %LIBDIR%\gd.lib

if x%freeglutdir% == x call :COPY %SRCDIR%\glut-3.7.6\libglutwin.lib %LIBDIR%\glut32.lib

set FREEGLUTLIBDIR=
if x%freeglutdir% == x goto skip_freeglut
set FREEGLUTLIBDIR=%BUILDDIR%\..\..\libs\freeglut\lib
call :COPY %FREEGLUTLIBDIR%\freeglut_static.lib %LIBDIR%\freeglut_static.lib
:skip_freeglut

call :COPY %SRCDIR%\pthreads\libpthreads.lib   %LIBDIR%\pthreads.lib
call :COPY %SRCDIR%\glui_v2_1_beta\libglui.lib %LIBDIR%\glui.lib

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
:CONT
if not exist %LIBDIR%\%file% timeout /t 1 > Nul
if not exist %LIBDIR%\%file% exit /b
timeout /t 1 > Nul
goto CONT

:eof
