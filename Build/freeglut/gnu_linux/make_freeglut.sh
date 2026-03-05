#!/bin/bash
#*** clean old files
echo "*** removing old files"
 git clean -dxf
 rm -rf  ../../../../libs/freeglut
 mkdir -p ../../../../libs/freeglut

#*** configure
echo "*** configuring"
 cmake ../../../../freeglut  -DCMAKE_INSTALL_PREFIX=../../../../libs/freeglut \
                          -DHAVE_X11_XF86VMODE=0 \
                          -DHAVE_X11_XRANDR=0
                          -DFREEGLUT_BUILD_GAMEMODE=OFF \
                          -DCMAKE_BUILD_TYPE=Release \
                          -DFREEGLUT_BUILD_DEMOS=OFF \
                          -DFREEGLUT_BUILD_SHARED_LIBS=OFF \
                          -DFREEGLUT_BUILD_STATIC_LIBS=ON

#*** build
echo "*** building"
make

#*** install
echo "*** installing"
make install

FREEGLUTFILE=../../../../libs/freeglut/lib64/libglut.a
if [ -e $FREEGLUTFILE ]; then
  ar -t $FREEGLUTFILE | awk '/gamemode/ {print $0}' | xargs -I{} ar d $FREEGLUTFILE {}
  ranlib $FREEGLUTFILE
  ar -t $FREEGLUTFILE | grep gamemode || echo "No gamemode objects found"
else
  echo ***error: freeglut file, $FREEGLUTFILE, does not exist
fi
