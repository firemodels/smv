#!/bin/bash
EXE=$1
input=$2

LOCKBASE=`whoami`_fdslock
LOCKFILE=/tmp/${input}_${LOCKBASE}$$

NPROCS=`grep processor /proc/cpuinfo | wc -l`
NJOBS=`ls -l /tmp/*${LOCKBASE}* | wc -l`

sleep 5
while [ $NJOBS -gt $NPROCS ]; do
  sleep 5
  NJOBS=`ls -l /tmp/*${LOCKBASE}* | wc -l`
done
sleep 5

touch $LOCKFILE
$EXE $input
rm -f $LOCKFILE
