#!/bin/bash
option=$1 $2 
if [ "$option" == "" ]; then
  option=-obst
fi

#dem2fds=dem2fds
dem2fds=../intel_linux_64/dem2fds_linux_64

terraindir=~/terrain

$dem2fds -dir $terraindir/gatlinburg test2.in 
$dem2fds -fds test2g.fds -geom -dir $terraindir/gatlinburg test2.in 
fds test2.fds 
fds test2g.fds
