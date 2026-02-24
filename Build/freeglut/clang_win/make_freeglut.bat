@echo off

::*** setup environment for clang-cl

if defined VSCMD_VER goto skip_setup
  set "VSINSTALL=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
  if defined VSCMD_USER set "VSINSTALL=%VSCMD_USER%"
  if exist "%VSINSTALL%\vcvars64.bat" goto skip_errorout
    echo ***error: clang-cl setup file
    echo           "%VSINSTALL%\vcvars64.bat"
    echo           does not exist. compilation aborted.
    exit /b
  :skip_errorout
  call "%VSINSTALL%\vcvars64.bat"
  if not defined VSCMD_VER echo ***error: clang-cl setup failed. compilation aborted.
  if not defined VSCMD_VER exit /b
:skip_setup
echo *** environment for clang-cl setup

::*** clean old files
echo *** removing old files
git clean -dxf
rmdir /s /q  ..\..\..\..\libs\freeglut
mkdir ..\..\..\..\libs\freeglut

::*** configure
echo *** configuring
cmake ../../../../freeglut ^
  -G "Ninja" ^
  -DCMAKE_INSTALL_PREFIX=../../../../libs/freeglut ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_C_COMPILER=clang-cl ^
  -DCMAKE_C_FLAGS="/utf-8" ^
  -DFREEGLUT_BUILD_SHARED_LIBS=OFF ^
  -DFREEGLUT_BUILD_STATIC_LIBS=ON ^
  -DFREEGLUT_BUILD_DEMOS=OFF

echo *** building
ninja

::*** install
echo *** installing
ninja install