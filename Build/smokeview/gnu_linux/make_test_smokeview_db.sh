#!/bin/bash
module load compilers/gcc4.9.2

LIBDIR=../../LIBS/gnu_linux/
source ../../scripts/test_libs.sh

SMV_MAKE_OPTS="SMV_TESTFLAG=\"-D pp_BETA\" "
SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_TESTSTRING=\"test_\" "

if [ "$BUILD_ALL" == "1" ]; then
  make -f ../Makefile clean
fi
eval make ${SMV_MAKE_OPTS} -f ../Makefile gnu_linux_db
