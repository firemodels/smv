#!/bin/bash
prog=$1
arg1=$2
arg2=$3
cat << EOF > file.$$
file $prog
run $arg1 $arg2
EOF
cat file.$$ | gdb
