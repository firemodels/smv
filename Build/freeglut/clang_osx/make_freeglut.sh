#!/bin/bash
#*** clean old files
echo "*** removing old files"
 git clean -dxf
 rm -rf  ../../../../libs/freeglut
 mkdir -p ../../../../libs/freeglut

#*** configure
echo "*** configuring"
 cmake ../../../../freeglut  -DCMAKE_INSTALL_PREFIX=../../../../libs/freeglut \
                          -DCMAKE_BUILD_TYPE=Release \
			  -DCMAKE_C_COMPILER=clang   \
                          -DFREEGLUT_COCOA=ON   \
                          -DFREEGLUT_BUILD_SHARED_LIBS=OFF \
                          -DFREEGLUT_BUILD_STATIC_LIBS=ON

#*** build
echo "*** building"
make

#*** install
echo "*** installing"
make install
