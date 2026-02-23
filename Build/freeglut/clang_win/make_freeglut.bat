@echo off

:: setup file
if not defined VSCMD_VER (
  for /f "usebackq tokens=*" %%i in (
    `"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" ^
     -latest ^
     -products * ^
     -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
     -property installationPath`
  ) do set "VSINSTALL=%%i"
  if "x%VSINSTALL%" == "x" echo ***error: clang-cl not installed or not available 
  if "x%VSINSTALL%" == "x" exit /b 
  if NOT exist "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" ***error: clang-cl not installed or not available  

  echo ***setting up environment for clang-cl
  call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" > Nul
)
if defined VSCMD_VER echo *** environment for clang-cl setup

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