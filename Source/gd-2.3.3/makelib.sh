#!/bin/bash
source ../../Build/scripts/setopts.sh $*
git clean -dxf
STDINT="-DHAVE_STDINT_H -DNONDLL -DENABLE_FREETYPE=OFF -DBUILD_SHARED_LIBS=OFF"
if [ "$COMPILER" == "icx" ]; then
  STDINT="$STDINT -Wno-deprecated-non-prototype"
fi
eval make STDINT=\"${STDINT}\" COMPILER=${COMPILER} SIZE=${SIZE} libgd.a
