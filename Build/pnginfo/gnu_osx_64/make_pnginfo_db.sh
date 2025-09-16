#!/bin/bash
curdir=`pwd`
LIBDIR=../../LIBS/gnu_osx_64_db/
if [ "$BUILD_LIBS" == "1" ]; then
  cd $LIBDIR
  ./make_LIBS.sh
  cd $curdir
else
  eval make -C ${LIBDIR} ${SMV_MAKE_OPTS} -f make_LIBS.make all
fi

rm -f *.o png_osx_64_db
make -f ../Makefile gnu_osx_64_db
