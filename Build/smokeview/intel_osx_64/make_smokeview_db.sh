#!/bin/bash
if [ "$NOQUARTZ" != "" ]; then
  OPTS="-Q $*"
  if [ "$LOWRES" != "" ]; then
    echo building smokeview no Quartz, low res
  else
    echo building smokeview no Quartz, high res
  fi
else
  echo building smokeview using Quartz
  OPTS="-q $*"
fi
source ../../scripts/setopts.sh $OPTS

if [ "$QUARTZSMV" == "use_quartz" ]; then
  LIBDIR=../../LIBS/intel_osx_64
else
  if [ "$LOWRES" == "" ]; then
    LIBDIR=../../LIBS/intel_osx_noq_64
  else
    LIBDIR=../../LIBS/intel_osx_noql_64
  fi
fi

CURDIR=`pwd`
cd $LIBDIR
build_libs=
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
if [ "$BUILD_LIBS" == "1" ]; then
  build_libs=1
fi
if [ "$build_libs" == "1" ]; then
  echo building libs
  eval ./make_LIBS.sh $OPTS
fi
cd $CURDIR

if [ "$BUILD_ALL" == "1" ]; then
  rm -f *.o *.mod smokeview_osx_64*
fi
eval make -j 4 QUARTZ="$QUARTZSMV" GLUT="$GLUT" ${SMV_MAKE_OPTS} -f ../Makefile intel_osx_64_db
