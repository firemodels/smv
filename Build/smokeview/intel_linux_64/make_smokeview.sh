#!/bin/bash
source ../../scripts/setopts.sh $*

# Exit immediately if any of the build steps fail
set -e

LIBDIR=../../LIBS/intel_linux_64/
eval make -C ${LIBDIR} ${SMV_MAKE_OPTS} ${LUA_SCRIPTING} -f make_LIBS.make all

make -f ../Makefile clean
eval make -j 4 ${SMV_MAKE_OPTS} -f ../Makefile intel_linux_64
