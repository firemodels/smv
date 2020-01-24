#!/bin/bash

#dem2fds=dem2fds
dem2fds=../intel_linux_64/dem2fds_linux_64

terraindir=~/terrain

$dem2fds $option       -geom -dir $terraindir/campfire campfire3.in
$dem2fds $option -show -geom -dir $terraindir/campfire campfire4.in
