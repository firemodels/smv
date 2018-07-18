#!/bin/bash
option=$1 $2 
if [ "$option" == "" ]; then
  option=-obst
fi

dem2fds=dem2fds
#dem2fds=../intel_linux_64/dem2fds_linux_64

terraindir=~/terrain

$dem2fds -dir $terraindir/gatlinburg test3.in 
$dem2fds -fds test3g.fds -geom -dir $terraindir/gatlinburg test3.in 
fds test3.fds 
fds test3g.fds 
