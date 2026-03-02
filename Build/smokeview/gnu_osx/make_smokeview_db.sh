#!/bin/bash
CURDIR=`pwd`
DIR=${CURDIR##*/}
OPTS="-G -l $DIR $*"

source ../../scripts/setopts.sh $OPTS

if [ "$BUILD_LIBS" == "1" ]; then
  rm -f $SMV_LIBDIR/*.a
fi
../../scripts/test_libs.sh $OPTS
if [ "$BUILD_ALL" == "1" ]; then
  rm -f *.o smokeview*
fi

eval make COMPILER=${COMPILER} COMPILER2=${COMPILER2} GLTYPE="$GLTYPE" GLUT="$GLUT" ${SMV_MAKE_OPTS} -f ../Makefile gnu_osx_db
