#!/bin/bash

OPTS="-Q $*"
echo building smokeview non/Quartz

source ../../scripts/setopts.sh $OPTS

LIBDIR=../../LIBS/intel_osx_64

CURDIR=`pwd`
cd $LIBDIR
build_libs=$BUILD_LIBS
LIBS="libgd.a libglui.a libjpeg.a libpng.a libz.a"

for f in $LIBS
do 
  if [ ! -e $f ]; then
    echo $f does not exist
    build_libs=1
  fi
done
if [ "$build_libs" != "" ]; then
  echo building libs
  eval ./make_LIBS.sh $OPTS
fi
cd $CURDIR

if [ "$BUILD_ALL" == "1" ]; then
  rm -f *.o *.mod smokeview_osx_64*
fi
eval make -j 4 GLUT="$GLUT" ${SMV_MAKE_OPTS} -f ../Makefile intel_osx_64_db
