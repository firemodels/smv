#!/bin/bash

# ---------------------------------------------------------------------------
# --------------------- beginning of script ---------------------------------
# ---------------------------------------------------------------------------

CURDIR=`pwd`
cd ../../..
export GITROOT=`pwd`
cd $CURDIR/..
export BASEDIR=`pwd`
PLATFORM=linux
COMPILER=intel
DEBUG=
TEST=
use_installed=
RUN_SMV=1
RUN_WUI=1
QUEUE=batch2
JOBPREFIX=SMV_

export SMV=$GITROOT/smv/Build/smokeview/${COMPILER}_$VERSION2/smokeview_$VERSION
export SMVBINDIR=$GITROOT/smv/Build/for_bundle

RUNSMV="$GITROOT/smv/Utilities/Scripts/qsmv.sh -j $JOBPREFIX $use_installed -q $QUEUE"
export QFDS=$RUNSMV
export RUNCFAST=$RUNSMV

echo Generating images

cd $GITROOT/smv/Verification
scripts/SMV_test.sh $QUEUE
cd $CURDIDR
