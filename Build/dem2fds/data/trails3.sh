#!/bin/bash

dem2fds=dem2fds
#dem2fds=../intel_linux_64/dem2fds_linux_64

terraindir=~/terrain

$dem2fds -obst -dir $terraindir/trails trails3.in 
$dem2fds -fds trails3g.fds -geom -dir $terraindir/trails trails3.in 
