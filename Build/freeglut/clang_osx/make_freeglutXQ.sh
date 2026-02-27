#!/bin/bash
#*** clean old files
echo "*** removing old files"
 git clean -dxf
 rm -rf  ../../../../libs/freeglut
 mkdir -p ../../../../libs/freeglut

#*** configure
echo "*** configuring"
XQ_PREFIX="/opt/X11"

cmake ../../../../freeglut \
  -G "Unix Makefiles" \
  -DCMAKE_INSTALL_PREFIX=../../../../libs/freeglut \
  -DCMAKE_BUILD_TYPE=Release \
  -DFREEGLUT_BUILD_DEMOS=OFF \
  -DCMAKE_C_COMPILER=clang \
  -DFREEGLUT_COCOA=OFF \
  -DFREEGLUT_X11=ON \
  -DX11_INCLUDE_DIR=${XQ_PREFIX}/include \
  -DX11_LIBRARIES=${XQ_PREFIX}/lib/libX11.dylib \
  -DCMAKE_PREFIX_PATH=${XQ_PREFIX} \
  -DCMAKE_EXE_LINKER_FLAGS="-L${XQ_PREFIX}/lib -Wl,-rpath,${XQ_PREFIX}/lib" \
  -DFREEGLUT_BUILD_SHARED_LIBS=OFF \
  -DFREEGLUT_BUILD_STATIC_LIBS=ON

#*** build
echo "*** building"
make

#*** install
echo "*** installing"
make install
