#!/bin/bash
source ../../scripts/setopts.sh $*

LIBDIR=../../LIBS/intel_osx_64
curdir=`pwd`
cd $LIBDIR
build_libs=
LIBS="libgd.a libglui.a libglut.a libjpeg.a libpng.a libz.a"
for f in $LIBS
do 
  if [ ! -e $f ]; then
    echo $f does not exist
    build_libs=1
  fi
done
if [ "$build_libs" == "1" ]; then
  cd $LIBDIR
  echo building libs
  eval ./make_LIBS.sh $*
  cd $curdir
fi

rm -f *.o *.mod smokeview_osx_64*
eval make -j 4 QUARTZ="$QUARTZ" GLUT="$GLUT" ${SMV_MAKE_OPTS} -f ../Makefile intel_osx_64_db
