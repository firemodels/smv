#!/bin/bash
OPTS="-G $*"
source ../../scripts/setopts.sh $OPTS

LIBDIR=../../LIBS/gnu_osx

CURDIR=`pwd`
cd $LIBDIR
build_libs=
LIBS="libgd.a libglui.a libjpeg.a libpng.a libz.a"

for f in $LIBS
do
  if [ ! -e $f ]; then
    build_libs=1
  fi
done
if [ "$BUILD_LIBS" == "1" ]; then
  build_libs=1
fi
if [ "$build_libs" == "1" ]; then
  echo building $LIBS
  eval ./make_LIBS.sh $OPTS
fi
cd $CURDIR

if [ "$BUILD_ALL" == "1" ]; then
  rm -f *.o *.mod smokeview_osx*
fi
eval make COMPILER=${COMPILER} COMPILER2=${COMPILER2} GLUT="$GLUT" ${SMV_MAKE_OPTS} -f ../Makefile gnu_osx_db
