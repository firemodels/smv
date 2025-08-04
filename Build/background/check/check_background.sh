#!/bin/bash

if [ "`uname`" == "Darwin" ] ; then
  platform="osx"
else
  platform="linux"
fi

eval make -j 8 $PLATFORM -f ../Makefile linux_osx_check
