#!/bin/bash
source ../../scripts/setopts.sh $*

make -f ../Makefile clean
eval make -j 4 ${SMV_MAKE_OPTS} -f ../Makefile intel_linux_64_db

