#!/bin/bash

#---------------------------------------------
#                   is_file_installed
#---------------------------------------------

is_file_installed()
{
  local program=$1

  $program  > prog_version 2>&1
  notfound=`cat prog_version | head -1 | grep "not found" | wc -l`
  rm prog_version
  if [ "$notfound" == "1" ] ; then
    echo 0
    exit
  fi
  echo 1
  exit
}
if [ "`uname`" == "Darwin" ]; then
  ICC="icc -diag-disable=10441"
else
  ICC=icx
fi
if [ "$INTEL_ICC" != "" ]; then
  ICC="$INTEL_ICC"
fi

icc_installed=`is_file_installed $ICC`
if [ "$icc_installed" == "0" ]; then
  echo Unknown
  exit
fi

$ICC -v  > icc_version 2>&1 
ICCVERSION=`cat icc_version | head -1`
rm icc_version
echo "\"$ICCVERSION\""
