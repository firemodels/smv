#!/bin/bash
OPTS="-G $*"

source ../../scripts/setopts.sh $OPTS

rm -f *.o
make -f ../Makefile clang_osx_64_db
