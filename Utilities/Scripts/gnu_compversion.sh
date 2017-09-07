#!/bin/bash

#---------------------------------------------
#                   is_file_installed
#---------------------------------------------

is_file_installed()
{
  local program=$1

  $program -help > prog_version 2>&1
  notfound=`cat prog_version | head -1 | grep "not found" | wc -l`
  rm prog_version
  if [ "$notfound" == "1" ] ; then
    echo 0
    exit
  fi
  echo 1
  exit
}

gcc_installed=`is_file_installed gcc`
if [ "$gcc_installed" == "0" ]; then
  echo unknown
  exit
fi

gcc -v > gcc_version 2>&1 
GCCVERSION=`cat gcc_version | awk '{print $3}' `
rm gcc_version
echo "\"Gnu C/C++ $GCCVERSION\""
