#!/bin/bash
source ../../scripts/setopts.sh $*

# Exit immediately if any of the build steps fail
set -e

LIBDIR=../../LIBS/gnu_linux_64/
eval make -C ${LIBDIR} -j 4 ${SMV_MAKE_OPTS} ${LUA_SCRIPTING} -f make_LIBS.make all

make -f ../Makefile clean
eval make ${SMV_MAKE_OPTS} -f ../Makefile gnu_linux_64
