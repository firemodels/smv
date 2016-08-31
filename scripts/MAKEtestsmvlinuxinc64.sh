#!/bin/bash
SVNROOT=~/$1

cd $SVNROOT/smv/Build/smokeview/intel_linux_64
./make_smv_inc.sh -t
