#!/bin/bash
OPTS="$*"
source ../../scripts/setopts.sh $OPTS

LIBDIR=../../LIBS/intel_osx_64
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
    build_libs=1
  fi
done
if [ "$build_libs" == "1" ]; then
  cd $LIBDIR
  echo building $LIBS
  eval ./make_LIBS.sh $OPTS
fi
cd $CURDIR

rm -f *.o *.mod smokeview_osx_64*
eval make -j 4 QUARTZ="$QUARTZSMV" GLUT="$GLUT" ${SMV_MAKE_OPTS} -f ../Makefile intel_osx_64_db
