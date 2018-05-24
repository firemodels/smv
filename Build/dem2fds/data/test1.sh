#!/bin/bash
option=$1 $2 
if [ "$option" == "" ]; then
  option=-obst
fi

#dem2fds=dem2fds
dem2fds=../intel_linux_64/dem2fds_linux_64

terraindir=~/terrain

$dem2fds $option -geom -dir $terraindir/gatlinburg test1.in 
$dem2fds $option -dir $terraindir/gatlinburg test2.in 
