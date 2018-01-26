#!/bin/bash
LIBDIR=../../LIBS/gnu_linux_32/
source ../../scripts/setopts.sh $*
source ../../scripts/test_libs.sh

make -f ../Makefile clean
eval make -j 4 ${SMV_MAKE_OPTS} -f ../Makefile gnu_linux_32

