#!/bin/bash
source ../../scripts/setopts.sh $*

curdir=`pwd`
LIBDIR=../../LIBS/intel_linux/

eval make -j 4 ${SMV_MAKE_OPTS} -f ../Makefile gnu_osx
