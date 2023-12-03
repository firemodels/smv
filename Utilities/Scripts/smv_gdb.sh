#!/bin/bash
ROOT=$1
prog=$ROOT/smv/Build/smokeview/gnu_linux_64/smokeview_linux_64_db
arg1=$2
arg2=$3
cat << EOF > file.$$
file $prog
run $arg1 $arg2
EOF
cat file.$$ | gdb
