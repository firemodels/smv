#!/bin/bash
CURDIR=`pwd`
DIR=${CURDIR##*/}
OPTS="-C -l $DIR $*"

source ../../scripts/setopts.sh $OPTS

if [ "$BUILD_LIBS" == "1" ]; then
  rm -f $SMV_LIBDIR/*.a
fi
../../scripts/test_libs.sh $OPTS
if [ "$BUILD_ALL" == "1" ]; then
  rm -f *.o pnginfo*
fi
eval make -j 4 ${SMV_MAKE_OPTS} -f ../Makefile gnu_linux_db
