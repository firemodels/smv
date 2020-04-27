#!/bin/bash
rm -f *.o
source ../scripts/setopts.sh $*


# If target has been specified, use that, otherwise use default
TARGET="${TARGET:-default}"

rm -f *.o
eval make GLUT=$GLUT COMPILER=${COMPILER} COMPILER2=${COMPILER2} CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\" ${TARGET}
