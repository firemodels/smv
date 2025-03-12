#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR
OPTS="-I $*"

source ../../../Source/scripts/setopts.sh $OPTS

LIBDIR=`pwd`
if [[ "$target" == "all" ]] || [[ "$target" == "clean" ]]; then
  rm *.a
  if [ "$target" == "clean" ]; then
    exit
  fi
fi

SRCDIR=$LIBDIR/../../../Source
cd $SRCDIR
SRCDIR=`pwd`

cd ../Build
BUILDDIR=`pwd`

if [[ "$target" == "all" ]] || [[ "$target" == "gd" ]]; then
# GD
echo "********** building GD"
cd $SRCDIR/gd-2.0.15
./makelib.sh $OPTS >& $LIBDIR/gd.out
cp libgd.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "glui" ]]; then
# GLUI
echo "********** building glui"
cd $SRCDIR/glui_v2_1_beta
./makelib.sh $OPTS  >& $LIBDIR/glui.out
cp libglui.a $LIBDIR/.
fi

# GLUT
if [[ "$target" == "all" ]] || [[ "$target" == "freeglut" ]] || [[ "$target" == "glut" ]]; then
if [ "$GLUT" == "freeglut" ]; then
echo "********** building freeglut"
  cd $BUILDDIR/freeglut3.0.0/intel_osx_64
  ./make_freeglut.sh $OPTS  >& $LIBDIR/freeglut.out
  cp libglut.a $LIBDIR/.
else
echo "********** using OSX provided glut"
fi
fi

if [[ "$target" == "all" ]] || [[ "$target" == "jpeg" ]]; then
# JPEG
echo "********** building jpeg"
cd $SRCDIR/jpeg-9b
./makelib.sh $OPTS >& $LIBDIR/jpeg.out
cp libjpeg.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "png" ]]; then
# PNG
echo "********** building png"
cd $SRCDIR/png-1.6.21
./makelib.sh $OPTS >& $LIBDIR/png.out
cp libpng.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "zlib" ]]; then
# ZLIB
echo "********** building zlib"
cd $SRCDIR/zlib128
./makelib.sh $OPTS >& $LIBDIR/zlib.out
cp libz.a $LIBDIR/.
fi
