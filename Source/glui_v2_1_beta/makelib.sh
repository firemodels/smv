#!/bin/bash
rm -f *.o
source ../scripts/setopts.sh $*


# If target has been specified, use that, otherwise use default
if [ "$TARGET" == "" ]; then
  TARGET=default
fi

rm -f *.o
eval make -j 4 GLUT=$GLUT COMPILER=${COMPILER} COMPILER2=${COMPILER2} CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\" $(TARGET)

