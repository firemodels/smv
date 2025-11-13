#!/bin/bash
ROOT=$1
prog=$ROOT/smv/Build/smokeview/gnu_linux/smokeview_linux_db
arg1=$2
arg2=$3
cat << EOF > file.$$
file $prog
run $arg1 $arg2
EOF
cat file.$$ | gdb
