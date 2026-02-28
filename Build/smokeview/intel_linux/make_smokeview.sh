#!/bin/bash
source ../../scripts/setopts.sh $*

curdir=`pwd`
LIBDIR=../../LIBS/intel_linux/
if [ "1" == "1" ]; then
  cd $LIBDIR
  ./make_LIBS.sh $*
  cd $curdir
fi
if [ "$BUILD_ALL" == "1" ]; then
  make -f ../Makefile clean
fi
eval make -j 4 ${SMV_MAKE_OPTS} -f ../Makefile intel_linux
