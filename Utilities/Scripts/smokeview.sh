#!/bin/bash

ssffile=
SMOKEVIEW=smokeview
RUNSCRIPT=-runscript
SMOKEVIEWDIR=$(dirname "$0")
TIME=
OUTPUT=

while getopts 'd:e:s:t' OPTION
do
case $OPTION in
  d)
   dir="$OPTARG"
   ;;
  e)
   SMOKEVIEW="$OPTARG"
   ;;
  s)
   ssffile=$OPTARG
   RUNSCRIPT="-scriptfile $OPTARG"
   ;;
  t)
  TIME=time 
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


echo $SMOKEVIEW $RUNSCRIPT $in
source $SMOKEVIEWDIR/startXserver.sh >/dev/null 2>&1
if [ "$TIME" == "time" ]; then
$TIME $SMOKEVIEW $RUNSCRIPT $in >/dev/null
else
$SMOKEVIEW $RUNSCRIPT $in
fi
source $SMOKEVIEWDIR/stopXserver.sh >/dev/null 2>&1
