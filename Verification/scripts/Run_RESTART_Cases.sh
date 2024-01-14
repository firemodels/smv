#!/bin/bash
CURDIR=`pwd`
cd ../../../fds
FDSREPO=`pwd`
QFDS=$FDSREPO/Utilities/Scripts/qfds.sh
cd $CURDIR/..

function usage {
echo "Run_SMV_Cases.sh [-d -h -m max_iterations -o nthreads -p -q queue -s ]"
echo "Runs Smokeview verification suite"
echo ""
echo "Options"
echo "-h - display this message"
echo "-j p - specify a job prefix"
echo "-q queue - run cases using the queue named queue"
exit
}

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

QUEUE=
JOBPREFIX=
while getopts 'hj:q:' OPTION
do
case $OPTION in
  h)
   usage;
   ;;
  j)
   JOBPREFIX="$OPTARG"
   ;;
  q)
   QUEUE="$OPTARG"
   ;;
esac
#shift
done

if [ "$QUEUE" != "" ]; then
  QUEUE="-q $QUEUE"
fi
if [ "$JOBPREFIX" != "" ]; then
  JOBPREFIX="RES"
fi
prefix=$JOBPREFIX
JOBPREFIX="-j $JOBPREFIX"

echo "running base cases"
echo ""
$QFDS -p 6 $JOBPREFIX $QUEUE -d Visualization geom1.fds
$QFDS -p 2 $JOBPREFIX $QUEUE -d Visualization test1.fds
$QFDS -p 6 $JOBPREFIX $QUEUE -d Visualization geom2.fds
$QFDS -p 2 $JOBPREFIX $QUEUE -d Visualization test2.fds

wait_cases_end $prefix
echo "running restart jobs"
echo ""
$QFDS -p 6 $JOBPREFIX $QUEUE -d Visualization geom1_restart.fds
$QFDS -p 2 $JOBPREFIX $QUEUE -d Visualization test1_restart.fds
