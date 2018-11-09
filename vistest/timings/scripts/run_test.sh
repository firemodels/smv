#!/bin/bash
CURDIR=`pwd`
cd ../../../
smvrepo=`pwd`
cd $CURDIR/../cases
source $smvrepo/Utilities/Scripts/startXserver.sh >/dev/null 2>&1
$smvrepo/Utilities/Scripts/smokeview.sh -t time1 -s plume_timing_iso.ssf     plume_timing
$smvrepo/Utilities/Scripts/smokeview.sh -t time2 -s plume_timing_3dsmoke.ssf plume_timing
$smvrepo/Utilities/Scripts/smokeview.sh -t time3 -s plume_timing_slice.ssf   plume_timing
$smvrepo/Utilities/Scripts/smokeview.sh -t time4 -s plume_timing_vslice.ssf  plume_timing
source $smvrepo/Utilities/Scripts/stopXserver.sh >/dev/null 2>&1
cd $CURDIR
