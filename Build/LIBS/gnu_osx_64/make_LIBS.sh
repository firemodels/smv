#!/bin/bash
# build freeglut if arg1==freeglut
arg1=$1

OPTS="-g"

LIBDIR=`pwd`

cd $LIBDIR/../../../Source
SRCDIR=`pwd`

cd ../Build
BUILDDIR=`pwd`

rm *.a
cd $SRCDIR
SRCDIR=`pwd`

# GD
cd $SRCDIR/gd-2.0.15
./makelib.sh $OPTS
cp libgd.a $LIBDIR/.

# GLUI
cd $SRCDIR/glui_v2_1_beta
if [ "$arg1" == "freeglut" ]; then
./makelib.sh $OPTS -f
else
./makelib.sh $OPTS
fi
cp libglui.a $LIBDIR/.

# FREEGLUT
if [ "$arg1" == "freeglut" ]; then
  cd $BUILDDIR/freeglut3.0.0/gnu_osx_64
  ./make_freeglut.sh $OPTS
  cp libglut.a $LIBDIR/.
fi

# JPEG
cd $SRCDIR/jpeg-9b
./makelib.sh $OPTS
cp libjpeg.a $LIBDIR/.

# PNG
cd $SRCDIR/png-1.6.21
./makelib.sh $OPTS
cp libpng.a $LIBDIR/.

# ZLIB
cd $SRCDIR/zlib128
./makelib.sh $OPTS
cp libz.a $LIBDIR/.
