@echo off


::*** clean old files
echo *** removing old files
git clean -dxf
if exist ..\..\..\..\libs\freeglut rmdir /s /q  ..\..\..\..\libs\freeglut
mkdir ..\..\..\..\libs\freeglut

::*** configure
echo *** configuring
cmake ../../../../freeglut ^
  -G "Ninja" ^
  -DCMAKE_INSTALL_PREFIX=../../../../libs/freeglut ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_C_COMPILER=clang-cl ^
  -DCMAKE_LINKER=link ^
  -DCMAKE_C_FLAGS="/utf-8 /O2 /Gy" ^
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded ^
  -DFREEGLUT_BUILD_SHARED_LIBS=OFF ^
  -DFREEGLUT_BUILD_STATIC_LIBS=ON ^
  -DFREEGLUT_BUILD_DEMOS=OFF
echo *** building
ninja

::*** install
echo *** installing
ninja install 
