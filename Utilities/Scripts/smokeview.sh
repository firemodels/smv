#!/bin/bash

PLATFORM=linux
if [ "`uname`" == "Darwin" ]; then
  PLATFORM="osx"
fi

ssffile=
module load SMV6
SMOKEVIEW=smokeview
RUNSCRIPT=-runscript
SMOKEVIEWDIR=$(dirname "$0")
TIME=
TIMEFILE=smv_time
OUTPUT=
SETUP_XSERVER=1
first=
skip=
ARG2=

while getopts 'd:e:F:ns:S:t:' OPTION
do
case $OPTION in
  d)
   dir="$OPTARG"
   ;;
  e)
   SMOKEVIEW="$OPTARG"
   ;;
  F)
   first="$OPTARG"
   ARG2=1
   ;;
  n)
  SETUP_XSERVER=
   ;;
  s)
   ssffile=$OPTARG
   RUNSCRIPT="-scriptfile $OPTARG"
   ;;
  S)
   skip="$OPTARG"
   ARG2=1
   ;;
  t)
  TIME=time 
  if [ "$PLATFORM" == "osx" ]; then
    TIME=gtime 
  fi
  TIMEFILE="$OPTARG"
  OUTPUT=">/dev/null"
   ;;
esac
done
shift $(($OPTIND-1))

in=$1
in=${in%*.*}

if [ "$ARG2" != "" ]; then
  if [ "$first" == "" ]; then
    first=1
  fi
  if [ "$skip" == "" ]; then
    skip=1
  fi
  ARG2="-startframe $start -skipframe $skip"
fi

if [ "$ssffile" == "" ]; then
  ssffile=$in.ssf
fi

notfound=`$SMOKEVIEW -help 2>&1 | tail -1 | grep "not found" | wc -l`
if [ "$notfound" == "1" ];  then
  echo "*** Error (fatal): The program $SMOKEVIEW is not available. Run aborted."
  exit
fi

if ! [ -e $in.smv ]; then
  echo "*** Error (fatal): The smokeview file, $in.smv, does not exist. Run aborted."
  exit
fi
if ! [ -e $ssffile ]; then
  echo "*** Error (fatal): The smokeview script file, $ssffile, does not exist. Run aborted."
  exit
fi

if [ "$TIME" != "" ]; then
  $TIME -p -o $TIMEFILE $SMOKEVIEW $RUNSCRIPT $ARG2 $in >/dev/null 
  grep real $TIMEFILE | awk -F' ' '{print $2}'
  rm -f $TIMEFILE
else
notfound=`xvfb-run 2>&1 >/dev/null | tail -1 | grep "not found" | wc -l`
if [ $notfound -eq 1 ]; then
   echo "***error: xvfb-run not installed"
else
  xvfb-run $SMOKEVIEW $RUNSCRIPT $ARG2 $in
fi
