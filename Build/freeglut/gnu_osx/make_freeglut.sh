#!/bin/bash
ARG=$1

FREEGLUTFILE=../../../../libs/freeglut/lib/libglut.a

#*** clean old files
echo "*** removing old files"
 git clean -dxf
 rm -rf  $FREEGLUT
 mkdir -p $FREEGLUT

#*** configure
echo "*** configuring"
if [[ "$GLTYPE" == "XQUARTZ" ]] || [[ "$ARG" == "XQUARTZ" ]]; then
  XQ_PREFIX="/opt/X11"

  cmake ../../../../freeglut \
  -G "Unix Makefiles" \
  -DFREEGLUT_BUILD_GAMEMODE=OFF \
  -DFREEGLUT_USE_XF86VM=OFF \
  -DCMAKE_INSTALL_PREFIX=../../../../libs/freeglut \
  -DCMAKE_BUILD_TYPE=Release \
  -DFREEGLUT_BUILD_DEMOS=OFF \
  -DFREEGLUT_COCOA=OFF \
  -DFREEGLUT_X11=ON \
  -DX11_INCLUDE_DIR=${XQ_PREFIX}/include \
  -DX11_LIBRARIES=${XQ_PREFIX}/lib/libX11.dylib \
  -DCMAKE_PREFIX_PATH=${XQ_PREFIX} \
  -DCMAKE_EXE_LINKER_FLAGS="-L${XQ_PREFIX}/lib -Wl,-rpath,${XQ_PREFIX}/lib" \
  -DFREEGLUT_BUILD_SHARED_LIBS=OFF \
  -DFREEGLUT_BUILD_STATIC_LIBS=ON
else
 cmake ../../../../freeglut -G "Unix Makefiles"  -DCMAKE_INSTALL_PREFIX=../../../../libs/freeglut \
                          -DCMAKE_BUILD_TYPE=Release \
                          -DFREEGLUT_BUILD_DEMOS=OFF \
                          -DFREEGLUT_COCOA=ON   \
                          -DFREEGLUT_BUILD_SHARED_LIBS=OFF \
                          -DFREEGLUT_BUILD_STATIC_LIBS=ON
fi

#*** build
echo "*** building"
make

#*** install
echo "*** installing"
make install

if [ -e $FREEGLUTFILE ]; then
  ar -t $FREEGLUTFILE | awk '/gamemode/ {print $0}' | xargs -I{} ar d $FREEGLUTFILE {}
  ranlib $FREEGLUTFILE
  ar -t $FREEGLUTFILE | grep gamemode || echo "No gamemode objects found"
else
  echo ***error: freeglut file, $FREEGLUTFILE, does not exist
fi
