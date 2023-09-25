#!/bin/bash
EXE=$1
input=$2

LOCKBASE=`whoami`_fdslock
LOCKFILE=/tmp/${input}_${LOCKBASE}$$
LSOUT=/tmp/lsout.$$

NPROCS=`grep processor /proc/cpuinfo | wc -l`
NJOBS=0
ls -l /tmp/*${LOCKBASE}* >& $LSOUT
if [ "`grep 'No such' $LSOUT`" == "" ]; then
  NJOBS=`ls -l /tmp/*${LOCKBASE}* | wc -l`
fi

echo "input=$input,NJOBS=$NJOBS, NPROCS=$NPROCS" >> /tmp/`whoami`_jobsummary
while [ $NJOBS -gt $NPROCS ]; do
  sleep 5
  NJOBS=0
  ls -l /tmp/*${LOCKBASE}* >& $LSOUT
  if [ "`grep 'No such' $LSOUT`" == "" ]; then
    NJOBS=`ls -l /tmp/*${LOCKBASE}* | wc -l`
  fi
echo "111input=$input,NJOBS=$NJOBS, NPROCS=$NPROCS" >> /tmp/`whoami`_jobsummary
done
sleep 5

rm $LSOUT
touch $LOCKFILE
$EXE $input
rm -f $LOCKFILE
