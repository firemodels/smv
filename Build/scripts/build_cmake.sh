#!bin/bash
CURDIR=`pwd`
cd ../..
cmake -B cbuild -DCMAKE_BUILD_TYPE=Release
cmake --build cbuild --config Release
cmake --install cbuild --config Release --prefix dist
cd $CURDIR