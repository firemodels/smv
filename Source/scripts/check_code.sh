#!/bin/bash
file=$1
base=${file%.*}
chkfile=${base}.chk
#CHECKS="-Xanalyzer -analyzer-checker=core.DivideZero"
CHECKS=
INC="-I ../smokeview -I ../glui_v2_1_beta -I ../shared -I ../glew -I . -I ../gd-2.0.15 -I ../zlib128"
clang --analyze $CHECKS $INC $file >& $chkfile
nwarnings=`tail -1 $chkfile | awk '{print $1}'`
if [ "$nwarnings" == "" ]; then
  nwarnings=0;
fi
echo $nwarnings warnings
