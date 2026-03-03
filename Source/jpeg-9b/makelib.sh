#!/bin/bash
git clean -dxf
source ../../Build/scripts/setopts.sh $*

eval make -j 4 COMPILER=${COMPILER} SIZE=${SIZE} libjpeg.a
