#!/bin/bash

PROG=$1
CASENAME=$2

LOCKBASE=`whoami`_fdslock
LOCKFILE=/tmp/${CASENAME}_${LOCKBASE}$$

NPROCS=`grep processors /proc/cpuinfo | wc -l`
NJOBS=`ls -l /tmp/*${LOCKFILES}* | wc -l`

while [ $NJOBS -GT $NPROCS ]; do
 sleep 10
done

sleep 1
touch $LOCKFILE
$PROG $CASENAME
rm $LOCKFILE
