#!/bin/bash

if [ "`uname`" == "Darwin" ] ; then
  platform="osx"
  PLATFORM="SMV_TESTFLAG=\"-D pp_OSX\""
else
  platform="linux"
  PLATFORM="SMV_TESTFLAG=\"-D pp_LINUX\""
fi

eval make -j 8 $PLATFORM -f ../Makefile linux_osx_check
