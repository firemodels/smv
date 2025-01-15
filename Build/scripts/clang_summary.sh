#!/bin/bash
OUTFILE=$1
echo > $OUTFILE
echo `pwd` >> $OUTFILE
for file in *.i; do
  echo "" >> $OUTFILE
  echo "**********************" >> $OUTFILE
  echo $file >> $OUTFILE
  echo "**********************" >> $OUTFILE
  cat $file >> $OUTFILE
done
