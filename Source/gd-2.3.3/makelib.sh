#!/bin/bash
source ../scripts/setopts.sh $*
rm -f *.o
STDINT="-DHAVE_STDINT_H -DNONDLL -DENABLE_FREETYPE=OFF -DBUILD_SHARED_LIBS=OFF"
eval make STDINT=\"${STDINT}\" COMPILER=${COMPILER} SIZE=${SIZE} libgd.a
