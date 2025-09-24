#!/bin/bash
source ../scripts/setopts.sh $*
rm -f *.o
STDINT="-DHAVE_STDINT_H"
eval make -j 4 STDINT=${STDINT} COMPILER=${COMPILER} SIZE=${SIZE} libgd.a
