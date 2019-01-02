#!/bin/bash
source ../../scripts/setopts.sh $*

LIBDIR=../../LIBS/gnu_osx_64
source ../../scripts/test_libs.sh $GLUT

make -f ../Makefile clean
eval make QUARTZ="$QUARTZ" GLUT="$GLUT" ${SMV_MAKE_OPTS} -f ../Makefile gnu_osx_64
