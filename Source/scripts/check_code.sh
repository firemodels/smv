#!/bin/bash
file=$1
base=${file%.*}
chkfile=${base}.chk
#CHECKS="-Xanalyzer -analyzer-checker=core.DivideZero"
CHECKS=
INC="-I ../smokeview -I ../glui_v2_1_beta -I ../shared -I ../glew -I . -I ../gd-2.0.15 -I ../zlib128"
clang --analyze $CHECKS $INC $file >& $chkfile
