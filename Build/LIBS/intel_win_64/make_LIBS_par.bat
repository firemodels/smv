@echo off
setlocal enabledelayedexpansion
set OPTS=i
set arg1=%1
set arg2=%2
:: if arg3 is freeglut then freeglut not glut library will be built
set arg3=%3

:: setup compiler environment
if x%arg1% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

set EXIT_SCRIPT=1

set LIBDIR=%CD%
git clean -dxf

cd ..\..\..\Source
set SRCDIR=%CD%

cd ..\Build
set BUILDDIR=%CD%

set libs=zlib128 zlib jpeg-9b jpeg png-1.6-21 png gd-2.0.15 gd glut-3.7.6 glutwin glui_v2_1_beta glui pthreads pthreads
set libs=zlib128 zlib jpeg-9b jpeg

set nargs=0
for %%x in ( %libs% ) do ( 
  set /A nargs+=1
  set libr[!nargs!]=%%~x
)
echo nargs=%nargs%

for /L %%i in (1,2,%nargs%) do (
  echo %%i
  echo !libr[%%i]!
  set /A ii=%%i
  set /A "iip1=%%i+1"
  echo ii=!ii! ip1=%iip1%
  echo args !libr[%%i]! !libr[%%iip1]!
  echo argi=%argi%
)

goto eof

::--------------------------------
:build_lib 
::--------------------------------
set libd=%1
set lib=%2
cd %SRCDIR%\%libd%
start "building windows %libd%"  makelib %OPTS% -copy %lib%.lib %LIBDIR%\%lib%.lib
exit /b

:eof

cd %LIBDIR%

if "x%arg1%" == "xbot" goto skip1
pause
dir *.lib
pause
:skip1
