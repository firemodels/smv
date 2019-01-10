#!/bin/bash
source ../../scripts/setopts.sh $*

make -f ../Makefile clean
eval make -f ../Makefile intel_linux_64
