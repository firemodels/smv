#!/bin/bash
EXE=$1
input=$2

LOCKBASE=`whoami`_fdslock
LOCKFILE=/tmp/${input}_${LOCKBASE}$$
touch $LOCKFILE
$EXE $input
rm -f $LOCKFILE
