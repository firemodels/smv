@echo off

:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_compilers.bat intel
if not defined ONEAPI_ROOT exit /b

::*** clean old files
echo *** removing old files
git clean -dxf
if exist ..\..\..\..\libs\freeglut rmdir /s /q  ..\..\..\..\libs\freeglut
mkdir ..\..\..\..\libs\freeglut

::*** configure
echo *** configuring
cmake ../../../../freeglut ^
-G Ninja ^
-DCMAKE_INSTALL_PREFIX=../../../../libs/freeglut ^
-DCMAKE_BUILD_TYPE=Release ^
-DCMAKE_C_COMPILER=icx ^
-DFREEGLUT_BUILD_SHARED_LIBS=OFF ^
-DFREEGLUT_BUILD_STATIC_LIBS=ON ^
-DFREEGLUT_BUILD_DEMOS=OFF ^
-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded ^
-DCMAKE_C_FLAGS_RELEASE="/O2"

echo *** building
ninja

::*** install
echo *** installing
ninja install