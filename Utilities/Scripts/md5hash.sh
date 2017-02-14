#!/bin/bash
filein=$1
fileout=$filein.md5

if [ -e $filein ]; then
   if [ "`uname`" == "Darwin" ] ; then
     hash=`cat $filein | md5`
     echo "$hash  $filein"  > $fileout
   else
     md5sum $filein > $fileout
   fi
else
  echo $filein does not exist
fi
