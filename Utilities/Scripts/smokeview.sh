#!/bin/bash

PLATFORM=linux
if [ "`uname`" == "Darwin" ]; then
  PLATFORM="osx"
fi

ssffile=
SMOKEVIEW=smokeview
RUNSCRIPT=-runscript
SMOKEVIEWDIR=$(dirname "$0")
TIME=
TIMEFILE=smv_time
OUTPUT=
SETUP_XSERVER=1

while getopts 'd:e:ns:t:' OPTION
do
case $OPTION in
  d)
   dir="$OPTARG"
   ;;
  e)
   SMOKEVIEW="$OPTARG"
   ;;
  n)
  SETUP_XSERVER=
   ;;
  s)
   ssffile=$OPTARG
   RUNSCRIPT="-scriptfile $OPTARG"
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


if [  "SETUP_XSERVER" == "1" ]; then
  source $SMOKEVIEWDIR/startXserver.sh >/dev/null 2>&1
fi
if [ "$TIME" != "" ]; then
  $TIME -p -o $TIMEFILE $SMOKEVIEW $RUNSCRIPT $in >/dev/null 
  grep real $TIMEFILE | awk -F' ' '{print $2}'
  rm -f $TIMEFILE
else
  $SMOKEVIEW $RUNSCRIPT $in
fi
if [  "SETUP_XSERVER" == "1" ]; then
  source $SMOKEVIEWDIR/stopXserver.sh >/dev/null 2>&1
fi
