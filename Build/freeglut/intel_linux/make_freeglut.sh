#!/bin/bash
FREEGLUTFILE=../../../../libs/freeglut/lib/libglut.a

#*** clean old files
echo "*** removing old files"
 git clean -dxf
 rm -rf  ../../../../libs/freeglut
 mkdir -p ../../../../libs/freeglut

#*** configure
echo "*** configuring"
cmake ../../../../freeglut -DCMAKE_INSTALL_PREFIX=../../../../libs/freeglut \
                           -DFREEGLUT_BUILD_GAMEMODE=OFF \
                           -DCMAKE_BUILD_TYPE=Release \
                           -DFREEGLUT_BUILD_DEMOS=OFF \
                           -DCMAKE_C_COMPILER=icx \
                           -DFREEGLUT_BUILD_SHARED_LIBS=OFF \
                           -DFREEGLUT_BUILD_STATIC_LIBS=ON

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
