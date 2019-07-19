#!/bin/bash
arg=$1
source ../../scripts/setopts.sh $*

LIBDIR=../../LIBS/gnu_linux_64/
source ../../scripts/test_libs.sh

if [ "$arg" != "-i" ]; then
  make -f ../Makefile clean
fi
eval make ${SMV_MAKE_OPTS} -f ../Makefile gnu_linux_64_db
