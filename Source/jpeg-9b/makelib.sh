#!/bin/bash
git clean -dxf
source ../scripts/setopts.sh $*

make -j 4 COMPILER="$COMPILER" SIZE="$SIZE" libjpeg.a
