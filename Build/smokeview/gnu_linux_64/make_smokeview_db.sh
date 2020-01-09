#!/bin/bash
source ../../scripts/setopts.sh $*

LIBDIR=../../LIBS/gnu_linux_64/
source ../../scripts/test_libs.sh

if [ "$inc" == "" ]; then
  make -f ../Makefile clean
fi
eval make -j 4 ${SMV_MAKE_OPTS} -f ../Makefile gnu_linux_64_db
