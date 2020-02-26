#/bin/bash

# This script runs the Smokeview Verification Cases on a 
# Linux machine with a batch queuing system

QUEUE=batch
DEBUG=
OPENMP_OPTS=
FDS_DEBUG=0
nthreads=1
RUN_SMV=1
RUN_WUI=1
STOPFDS=
CFASTREPO=~/cfastgitclean
COMPILER="intel"
WAIT=0
INTEL=
INTEL2=
QFDS_COUNT=/tmp/qfds_count_`whoami`

wait_cases_end()
{
   if [[ "$QUEUE" == "none" ]]
   then
     while [[ `ps -u $USER -f | fgrep .fds | grep -v grep` != '' ]]; do
        JOBS_REMAINING=`ps -u $USER -f | fgrep .fds | grep -v grep | wc -l`
        echo "Waiting for ${JOBS_REMAINING} cases to complete."
        sleep 15
     done
   else
     while [[ `qstat -a | awk '{print $2 $4 $10}' | grep $(whoami) | grep $JOBPREFIX | grep -v 'C$'` != '' ]]; do
        JOBS_REMAINING=`qstat -a | awk '{print $2 $4 $10}' | grep $(whoami) | grep $JOBPREFIX | grep -v 'C$' | wc -l`
        echo "Waiting for ${JOBS_REMAINING} cases to complete." 
        sleep 15
     done
   fi
}

function usage {
echo "Run_SMV_Cases.sh [-d -h -m max_iterations -o nthreads -p -q queue -s ]"
echo "Runs Smokeview verification suite"
echo ""
echo "Options"
echo "-c - cfast repo directory"
echo "-d - use debug version of FDS"
echo "-h - display this message"
echo "-I - compiler (intel or gnu)"
echo "-j p - specify a job prefix"
echo "-J - use Intel MPI version of FDS"
echo "-m max_iterations - stop FDS runs after a specifed number of iterations (delayed stop)"
echo "     example: an option of 10 would cause FDS to stop after 10 iterations"
echo "-o nthreads - run OpenMP version of FDS with a specified number of threads [default: $nthreads]"
echo "-q queue - run cases using the queue named queue"
echo "     default: batch"
echo "     other options: vis"
echo "-r - run only regular smokeview cases"
echo "-s - stop FDS runs"
echo "-u - use installed versions of utilities background and wind2fds"
echo "-w - wait for cases to complete before returning"
echo "-W - run only WUI cases"
echo "-Y - run SMV and WUI cases"
exit
}

is_file_installed()
{
  program=$1
  notfound=`$program -help 2>&1 | tail -1 | grep "not found" | wc -l`
  if [ "$notfound" == "1" ] ; then
    echo "***error: $program not available. Run aborted." 
    exit
  fi
}


CURDIR=`pwd`
cd ..

SVNROOT=`pwd`/../..
cd $SVNROOT
SVNROOT=`pwd`

cd $CURDIR/..

use_installed="0"
while getopts 'c:dhI:j:Jm:o:q:rsS:uWwY' OPTION
do
case $OPTION in
  c)
   CFASTREPO="$OPTARG"
   ;;
  d)
   DEBUG=_db
   FDS_DEBUG=1
   ;;
  h)
   usage;
   ;;
  I)
   COMPILER="$OPTARG"
   ;;
  j)
   JOBPREFIX="$OPTARG"
   ;;
  J)
   INTEL=i
   INTEL2="-I"
   ;;
  m)
   export STOPFDSMAXITER="$OPTARG"
   ;;
  o)
   nthreads="$OPTARG"
   OPENMP_OPTS="-n $nthreads"
   ;;
  q)
   QUEUE="$OPTARG"
   ;;
  r)
   RUN_SMV=1
   ;;
  s)
   stop_cases=true
   export STOPFDS=-s
   ;;
  u)
   use_installed="1"
   ;;
  w)
   WAIT="1"
   ;;
  W)
   RUN_SMV=0
   RUN_WUI=1
   ;;
  Y)
   RUN_SMV=1
   RUN_WUI=1
esac
#shift
done

export FDS_DEBUG

if [ "$JOBPREFIX" == "" ]; then
  JOBPREFIX=SB_
fi

OS=`uname`
if [ "$OS" == "Darwin" ]; then
  PLATFORM=osx_64
else
  PLATFORM=linux_64
fi

if [ "$use_installed" == "1" ] ; then
  export WIND2FDS=wind2fds
  export BACKGROUND_PROG=background
else
  export WIND2FDS=$SVNROOT/smv/Build/wind2fds/${COMPILER}_$PLATFORM/wind2fds_$PLATFORM
  export BACKGROUND_PROG=$SVNROOT/smv/Build/background/${COMPILER}_$PLATFORM/background_$PLATFORM
fi
export FDSEXE=$SVNROOT/fds/Build/${INTEL}mpi_${COMPILER}_$PLATFORM$DEBUG/fds_${INTEL}mpi_${COMPILER}_$PLATFORM$DEBUG
export FDS=$FDSEXE
export FDSMPI=$SVNROOT/fds/Build/${INTEL}mpi_${COMPILER}_$PLATFORM$DEBUG/fds_${INTEL}mpi_${COMPILER}_$PLATFORM$DEBUG
export CFAST=$CFASTREPO/Build/CFAST/${COMPILER}_$PLATFORM/cfast7_$PLATFORM
QFDSSH="$SVNROOT/smv/Utilities/Scripts/qfds.sh -j $JOBPREFIX"

# Set queue to submit cases to

if [ "$QUEUE" != "" ]; then
   if [ "$QUEUE" == "none" ]; then
      is_file_installed $BACKGROUND_PROG
      echo 0 > $QFDS_COUNT
   fi
   QUEUE="-q $QUEUE"
fi

export BASEDIR=`pwd`

# Remove output files (unless stop option is used)
if [[ ! $stop_cases ]] ; then
  echo "Removing FDS/CFAST output files"
  export RUNCFAST="$SVNROOT/smv/Verification/scripts/Remove_CFAST_Files.sh"
  export QFDS="$SVNROOT/fds/Verification/scripts/Remove_FDS_Files.sh"
  scripts/SMV_Cases.sh
  scripts/WUI_Cases.sh
  echo "FDS/CFAST output files removed"
fi

# run cases    

export  RUNCFAST="$QFDSSH $INTEL2 -e $CFAST $QUEUE $STOPFDS"
export      QFDS="$QFDSSH $INTEL2 -e $FDSEXE $OPENMPOPTS $QUEUE $STOPFDS"

echo "" | $FDSEXE 2> $SVNROOT/smv/Manuals/SMV_User_Guide/SCRIPT_FIGURES/fds.version

if [[ ! $stop_cases ]] ; then
  if [ "$FDS_DEBUG" == "0" ] ; then
    if [ "$RUN_WUI" == "1" ] ; then
      is_file_installed $WIND2FDS
      cd $SVNROOT/smv/Verification/WUI
      echo Converting wind data
      $WIND2FDS -prefix sd11 -offset " 100.0  100.0 0.0" wind_data1a.csv
    fi
  fi
fi

if [ "$RUN_SMV" == "1" ] ; then
  cd $SVNROOT/smv/Verification
  scripts/SMV_Cases.sh
fi
if [ "$RUN_WUI" == "1" ] ; then
  cd $SVNROOT/smv/Verification
  scripts/WUI_Cases.sh
fi
if [ "$WAIT" == "1" ] ; then
  wait_cases_end
fi

echo FDS cases submitted

