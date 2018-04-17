#!/bin/bash
source ../../scripts/setopts.sh $*

LIBDIR=../../LIBS/gnu_linux_64/
source ../../scripts/test_libs.sh

make -f ../Makefile clean
eval make ${SMV_MAKE_OPTS} -f ../Makefile gnu_linux_64
