#!/bin/bash
source ../../scripts/setopts.sh $*
LIBDIR=../../LIBS/intel_osx_64/
source ../../scripts/test_libs.sh
source $IFORT_COMPILER/bin/compilervars.sh intel64

make -f ../Makefile clean
eval make -j 4 ${SMV_MAKE_OPTS}-f ../Makefile intel_osx_64
