#!/bin/bash
curdir=`pwd`
cpufrom=$HOME/.firebot/fds_times.csv
historydir=$HOME/.firebot/history
FILELIST="*benchmark*csv"
tempfile=/tmp/filelist.$$
# The offset below is computed by substituting
# Jan 1, 2016 5 UTC (12 AM EST) into a web form
# found at:
# http://www.unixtimestamp.com/
BASETIMESTAMP=1451624400
CURDIR=`pwd`
cd ../../smv
while getopts 's' OPTION
do
case $OPTION  in
  s)
   cpufrom=$HOME/.smokebot/smv_times.csv
   historydir=$HOME/.smokebot/history
   FILELIST="*csv"
   ;;
esac
done
shift $(($OPTIND-1))
for file in $historydir/$FILELIST
do
time=`tail -1 $file`
hash=`tail -2 $file | head -1`
gitdate=`git show -s --format=%ct $hash`
gitdate=`echo "scale=5; $gitdate - $BASETIMESTAMP" | bc`
gitdate=`echo "scale=5; $gitdate/86400 " | bc`
echo $gitdate,$time>> $tempfile
done
sort -t ',' -n -k 1 $tempfile
rm $tempfile
cd $CURDIR
