#!/bin/bash
CURDIR=`pwd`

cd ../../../fds/Manuals/FDS_User_Guide
FDSUGDIR=`pwd`
cd $CURDIR
FILE=$FDSUGDIR/FDS_User_Guide.tex
TEMP=/tmp/temp.$$

cat $FILE  | sed -e '1,/caption\[Freq/d' | \
             sed -e '/end{longtable/,$d' | \
             sed -e '1,/endhead/d' | \
             awk -F'&' '{print $1 " & " $4}' | \
             sed 's/{\\ct //g' | \
             sed 's/\\\\//g' | \
             sed 's/\\hline//g' |
             sed -r 's/}.*\&/   \&/g' |\
             sed 's/\\_/_/g' >& $TEMP

echo BNDFs
cat $TEMP    | awk -F'&' '$2 ~ /.*B.*/ { print $1 }'
echo
echo SLCFs
cat $TEMP    | awk -F'&' '$2 ~ /.*S.*/ { print $1 }'

