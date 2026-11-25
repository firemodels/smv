@echo off
set COMPILER_TYPE=%1
set freeglutdir=%2
set build_wait=
if "%SMV_WAIT_FOR_LIBS%" == "1" set build_wait=/b /wait

set LIBDIR=%CD%
echo *** Cleaning %CD%
git clean -dxf > Nul

cd ..\..\..\Source\LIBS
set SRCDIR=%CD%

cd ..\..\Build
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
call :BUILD zlib
      
:: JPEG
cd %SRCDIR%\jpeg-9b
echo *** building jpeg
call :BUILD jpeg

:: PNG
cd %SRCDIR%\png-1.6.48
echo *** building png
call :BUILD png

:: GD
cd %SRCDIR%\gd-2.3.3
echo *** building gd
call :BUILD gd

:: pthreads
cd %SRCDIR%\pthreads
echo *** building pthreads
call :BUILD pthreads

:: GLUT%
if NOT x%freeglutdir% == x goto skip_glut
  endlocal
  if %COMPILER_TYPE% == i call %UTILDIR%\Scripts\setup_intel_compilers.bat
  if %COMPILER_TYPE% == c call %UTILDIR%\Scripts\setup_clang_compilers.bat force
  cd %SRCDIR%\glut-3.7.6
  echo *** building glut
  start "building glut" /WAIT cmd /c "makelib %COMPILER_TYPE%  > %LIBDIR%\glut.out 2>&1"
  echo *** glut build complete
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
call :BUILD glui %freeglutdir%

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
setlocal
set "lib_copy_failed="

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

if defined lib_copy_failed exit /b 1
exit /b 0

::------------------------------------------------
:COPY
::------------------------------------------------
set label=%~n1%~x1
set infile=%1
set outfile=%2
IF NOT EXIST %infile% goto else1
   echo *** copying %label%
   copy %infile% %outfile% >Nul || set "lib_copy_failed=1"
   goto endif1
:else1
   echo.
   echo *** warning: %infile% does not exist
   set "lib_copy_failed=1"
   echo.
:endif1
exit /b

::------------------------------------------------
:BUILD
::------------------------------------------------
:: Mark the job before launching it so :WAIT cannot miss a worker starting up.
type Nul > "%LIBDIR%\%1.running"
start "building %1" %build_wait% cmd /d /c call "%COMMON%\lib_wrapper.bat" "%1" "%LIBDIR%" makelib %COMPILER_TYPE% %2 || (
  del "%LIBDIR%\%1.running"
  echo *** warning: could not start %1 build
)
exit /b

::------------------------------------------------
:WAIT
::------------------------------------------------
if not exist "%LIBDIR%\%1" exit /b
:: timeout requires console input, which the GUI redirects.
powershell.exe -NoProfile -NonInteractive -Command "$marker = Join-Path $env:LIBDIR '%1'; while (Test-Path -LiteralPath $marker) { Start-Sleep -Milliseconds 250 }"
exit /b
