#!/bin/bash
source ../scripts/setopts.sh $*

rm -f *.o
eval make -j 4 COMPILER=${COMPILER} SIZE=${SIZE} libtiff.a
