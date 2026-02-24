@echo off

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_compilers.bat clang

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
