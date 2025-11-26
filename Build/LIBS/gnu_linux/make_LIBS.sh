#!/bin/bash
# use -G to force use of the gnu compiler
OPTS="-G $*"
source ../../../Source/scripts/setopts.sh $OPTS

LIBDIR=`pwd`
rm $LIBDIR/*.a

cd $LIBDIR/../../../Source
SRCDIR=`pwd`

cd ../Build
BUILDDIR=`pwd`

# GD
cd $SRCDIR/gd-2.3.3
./makelib.sh $OPTS
cp libgd.a $LIBDIR/.

# GLUI
cd $SRCDIR/glui_v2_1_beta
./makelib.sh $OPTS
cp libglui.a $LIBDIR/.

# GLUT
if [ "$GLUT" == "freeglut" ]; then
  cd $BUILDDIR/freeglut3.0.0/gnu_linux
  ./make_freeglut.sh $OPTS
else
  cd $SRCDIR/glut-3.7.6
  ./makelib.sh $OPTS
fi
cp libglut.a $LIBDIR/.

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
