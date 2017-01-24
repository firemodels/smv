#!/bin/bash
filein=$1
fileout=$filein.md5

if [ -e $filein ]; then
   if [ "`uname`" == "Darwin" ] ; then
     cat $filein | md5sum > $fileout
   else
     md5sum $filein > $fileout
   fi
else
  echo $filein does not exist
fi
