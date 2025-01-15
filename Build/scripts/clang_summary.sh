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
  exit
fi
for file in *.chk; do
  if [ -s $file ]; then
    nwarnings=`tail -1 $file | awk '{print $1}'`
    echo $file: $nwarnings >> $OUTFILE
  fi
done
