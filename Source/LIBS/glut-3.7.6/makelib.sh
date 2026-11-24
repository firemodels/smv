#!/bin/bash
git clean -dxf
source ../scripts/setopts.sh $*

eval make -j 4 COMPILER=${COMPILER} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\" ${TARGET}
