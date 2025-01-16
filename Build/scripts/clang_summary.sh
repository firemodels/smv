#!/bin/bash
OUTFILE=$1
if [ "$OUTFILE" == "" ]; then
  OUTFILE=clang_warnings.txt
fi
HAVE_WARNINGS=
for file in *.chk; do
  if [ -s $file ]; then
    HAVE_WARNINGS=1
    break
  fi
done
echo clang warnings > $OUTFILE
echo -------------- >> $OUTFILE
if [ "$HAVE_WARNINGS" == "" ]; then
  echo no warnings were found > $OUTFILE
fi
total_warnings=0
nfiles=0
for file in *.chk; do
  nfiles=$((nfiles+1))
  if [ -s $file ]; then
    nwarnings=`tail -1 $file | awk '{print $1}'`
    total_warnings=$((total_warnings+nwarnings))
    echo $file: $nwarnings >> $OUTFILE
  fi
done
echo
echo $nfiles files scanned, $total_warnings warnings found

