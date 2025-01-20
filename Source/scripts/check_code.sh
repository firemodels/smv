#!/bin/bash
file=$1
option=$2

if [ ! -e $file ]; then
  echo *error: $file does not exist
  exit
fi
base=${file%.*}
chkfile=${base}.chk
CHECKS=
if [ "$option" == "-t" ]; then
  CHECKS="-Xanalyzer -analyzer-checker=core,deadcode -D pp_CLANG_TEST"
fi
INC="-I ../smokeview -I ../glui_v2_1_beta -I ../shared -I ../glew -I . -I ../gd-2.0.15 -I ../zlib128"
clang --analyze $CHECKS $INC $file >& $chkfile
nwarnings=`tail -1 $chkfile | awk '{print $1}'`
if [ "$nwarnings" == "" ]; then
  nwarnings=0
fi
echo "$file: $nwarnings warnings"
