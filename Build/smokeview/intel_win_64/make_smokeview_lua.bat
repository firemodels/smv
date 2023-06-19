@echo off

call ..\..\scripts\set_smv_opts %*

:: setup compiler environment
if x%from% == xbot goto skip1
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
:skip1

cd ..\..\..\
cmake -B cbuild -DLUA=ON -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build cbuild --config Release
cmake --install cbuild --config Release --prefix dist
ctest --test-dir cbuild -C Release
