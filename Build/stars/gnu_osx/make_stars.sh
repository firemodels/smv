#!/bin/bash
source ../../scripts/setopts.sh $*

# Exit immediately if any of the build steps fail
set -e

curdir=`pwd`
LIBDIR=../../LIBS/intel_linux/

eval make -j 4 ${SMV_MAKE_OPTS} -f ../Makefile gnu_osx
