#!/bin/bash
chid=$1

casename=${chid}.fds
running=${chid}.running
stopfile=${chid}.stop
if [ -e $running ]; then
  echo $casename already running
  exit 1
fi
echo 1 > $running
rm -f $stopfile
fds $casename
rm -f $running
