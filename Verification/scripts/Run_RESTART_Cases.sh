#!/bin/bash
JPREFIX=$1
if [ "$JPREFIX" == "" ]; then
  JPREFIX="RES"
fi
PREFIX="-j $JPREFIX"
CURDIR=`pwd`
cd ../../../fds
FDSREPO=`pwd`
QFDS=$FDSREPO/Utilities/Scripts/qfds.sh
cd $CURDIR/..

wait_cases_end()
{
   prefix=$1
   # Scans qstat and waits for verification cases to end
   if [[ "$QUEUE" == "none" ]]
   then
     while [[          `ps -u $USER -f | fgrep .fds | grep -v smokebot | grep -v grep` != '' ]]; do
        JOBS_REMAINING=`ps -u $USER -f | fgrep .fds | grep -v smokebot | grep -v grep | wc -l`
        sleep 30
     done
   else
     while           [[ `qstat -a | awk '{print $2 $4 $10}' | grep $(whoami) | grep $prefix | grep -v 'C$'` != '' ]]; do
        JOBS_REMAINING=`qstat  -a | awk '{print $2 $4 $10}' | grep $(whoami) | grep $prefix | grep -v 'C$' | wc -l`
        sleep 30
     done
   fi
}

echo "running base cases"
echo ""
$QFDS -p 6 $PREFIX -d Visualization geom1.fds
$QFDS -p 2 $PREFIX -d Visualization test1.fds
$QFDS -p 6 $PREFIX -d Visualization geom2.fds
$QFDS -p 2 $PREFIX -d Visualization test2.fds

wait_cases_end $JPREFIX
echo "running restart jobs"
echo ""
$QFDS -p 6 $PREFIX -d Visualization geom1_restart.fds
$QFDS -p 2 $PREFIX -d Visualization test1_restart.fds
