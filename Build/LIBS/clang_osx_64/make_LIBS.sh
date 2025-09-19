#!/bin/bash
# use -C to force use of the clang compiler
OPTS="-C $*"

source ../../../Source/scripts/setopts.sh $OPTS

LIBDIR=`pwd`
rm *.a

cd $LIBDIR/../../../Source
SRCDIR=`pwd`

cd ../Build
BUILDDIR=`pwd`

cd $SRCDIR
SRCDIR=`pwd`

# GD
cd $SRCDIR/gd-2.0.15
./makelib.sh $OPTS
cp libgd.a $LIBDIR/.

# GLUI
cd $SRCDIR/glui_v2_1_beta
if [ "$GLUT" == "freeglut" ]; then
  ./makelib.sh $OPTS -f
else
  ./makelib.sh $OPTS
fi
cp libglui.a $LIBDIR/.

# FREEGLUT
if [ "$GLUT" == "freeglut" ]; then
  cd $BUILDDIR/freeglut3.0.0/clang_osx_64
  ./make_freeglut.sh $OPTS
  cp libglut.a $LIBDIR/.
else
  echo ***using OSX provided glut
fi

# JPEG
cd $SRCDIR/jpeg-9b
./makelib.sh $OPTS
cp libjpeg.a $LIBDIR/.

# PNG
cd $SRCDIR/png-1.6.48
./makelib.sh $OPTS
cp libpng.a $LIBDIR/.

# ZLIB
cd $SRCDIR/zlib131
./makelib.sh $OPTS
cp libz.a $LIBDIR/.
