#!/bin/bash
source ../../scripts/setopts.sh $*

LIBDIR=../../LIBS/gnu_osx_64/
eval make -C ${LIBDIR} ${SMV_MAKE_OPTS} ${LUA_SCRIPTING} -f make_LIBS.make all_nolua_noglut

make -f ../Makefile clean
eval make ${SMV_MAKE_OPTS} -f ../Makefile gnu_osx_64_db
