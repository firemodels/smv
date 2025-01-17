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
nwarnfiles=0
echo ""
for file in *.chk; do
  nfiles=$((nfiles+1))
  if [ -s $file ]; then
    nwarnings=`tail -1 $file | awk '{print $1}'`
    total_warnings=$((total_warnings+nwarnings))
    nwarnfiles=$((nwarnfiles+1))
    echo $file: $nwarnings  warnings >> $OUTFILE
    if [ "$nwarnings" != "0" ]; then
      echo $file: $nwarnings warnings
    fi
  fi
done
echo
if [ "$total_warnings" == "0" ]; then
  echo $nfiles files scanned, no warnings were found
else
  echo $nfiles files scanned, $nwarnfiles files have $total_warnings warnings
fi

