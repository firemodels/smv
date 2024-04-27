#!/bin/bash
casename=$1
running=${casename}.running
if [ -e $running ]; then
  echo $casename already running
  exit 1
fi
echo 1 > $running
fds $casename
rm -f $running