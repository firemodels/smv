#!/bin/bash
source ../../scripts/setopts.sh $*

make -f ../Makefile clean
eval make -j 4 QUARTZ="$QUARTZ" GLUT="$GLUT" ${SMV_MAKE_OPTS}-f ../Makefile intel_osx_64
