#!/bin/bash
dir=$1
command=$2
arg=$3

cd $dir
./$command $arg
