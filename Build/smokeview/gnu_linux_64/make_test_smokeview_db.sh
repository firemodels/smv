#!/bin/bash
module load compilers/gcc4.9.2

LIBDIR=../../LIBS/gnu_linux_64/
source ../../scripts/test_libs.sh

SMV_MAKE_OPTS="SMV_TESTFLAG=\"-D pp_BETA\" "
SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_TESTSTRING=\"test_\" "

make -f ../Makefile clean
eval make ${SMV_MAKE_OPTS} -f ../Makefile gnu_linux_64_db
