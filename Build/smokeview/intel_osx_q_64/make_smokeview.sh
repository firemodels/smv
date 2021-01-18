#!/bin/bash
echo building smokeview using Quartz
OPTS="-q $*"

source ../../scripts/setopts.sh $OPTS

if [ "$QUARTZSMV" == "use_quartz" ]; then
  LIBDIR=../../LIBS/intel_osx_q_64
else
  LIBDIR=../../LIBS/intel_osx_64
fi

CURDIR=`pwd`
cd $LIBDIR
build_libs=$BUILD_LIBS
LIBS="libgd.a libglui.a libjpeg.a libpng.a libz.a"

# build glut if using the quartz library
if [ "$QUARTZSMV" == "use_quartz" ]; then
  LIBS="$LIBS libglut.a"
fi

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
eval make -j 4 QUARTZ="$QUARTZSMV" GLUT="$GLUT" ${SMV_MAKE_OPTS} -f ../Makefile intel_osx_64
