#!/bin/bash
CURDIR=`pwd`
cd ../cases
../../../Utilities/Scripts/smokeview.sh -t -s plume_timing_iso.ssf plume_timing
../../../Utilities/Scripts/smokeview.sh -t -s plume_timing_3dsmoke.ssf plume_timing
../../../Utilities/Scripts/smokeview.sh -t -s plume_timing_slice.ssf plume_timing
../../../Utilities/Scripts/smokeview.sh -t -s plume_timing_vslice.ssf plume_timing
cd $CURDIR
