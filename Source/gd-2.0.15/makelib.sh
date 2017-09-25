#!/bin/bash
source ../scripts/setopts.sh $*
rm -f *.o
STDINT="-DHAVE_STDINT_H"
eval make STDINT=${STDINT} COMPILER=${COMPILER} SIZE=${SIZE} libgd.a
