#!/bin/bash
CURDIR=`pwd`
SMOKEVIEW=
LABEL=smokeview
SETUP_XSERVER=1
CASE=plume_timing 
CASEDIR=$CURDIR/../cases
TIMEFILE=timefile.$$

while getopts 'c:C:e:l:n' OPTION
do
case $OPTION in
  c)
   CASE="$OPTARG"
   ;;
  C)
   CASEDIR="$OPTARG"
   ;;
  e)
   SMOKEVIEW="$OPTARG"
   ;;
  l)
   LABEL="$OPTARG"
   ;;
  n)
  SETUP_XSERVER=
   ;;
esac
done
shift $(($OPTIND-1))

if [ "$SMOKEVIEW" != "" ]; then
  if [ -e $SMOKVIEW ]; then
    SMOKEVIEW="-e $SMOKEVIEW"
  else
    exit
  fi
fi

cd ../../../
smvrepo=`pwd`
if [ ! -d $CASEDIR ]; then
  echo "***error the directory $CASEDIR does not exist"
  exit
fi
cd $CASEDIR
if [ "$SETUP_XSERVER" == "1" ]; then
  source $smvrepo/Utilities/Scripts/startXserver.sh >/dev/null 2>&1
fi
time0=0.0
time1=0.0
time2=0.0
time3=0.0
time4=0.0
if [ -e ${CASE}_startup.ssf ]; then
  time0=`$smvrepo/Utilities/Scripts/smokeview.sh $SMOKEVIEW -t $TIMEFILE -s ${CASE}_startup.ssf $CASE`
fi
if [ -e ${CASE}_iso.ssf ]; then
  time1=`$smvrepo/Utilities/Scripts/smokeview.sh $SMOKEVIEW -t $TIMEFILE -s ${CASE}_iso.ssf     $CASE`
fi
if [ -e ${CASE}_3dsmoke.ssf ]; then
  time2=`$smvrepo/Utilities/Scripts/smokeview.sh $SMOKEVIEW -t $TIMEFILE -s ${CASE}_3dsmoke.ssf $CASE`
fi
if [ -e ${CASE}_slice.ssf ]; then
  time3=`$smvrepo/Utilities/Scripts/smokeview.sh $SMOKEVIEW -t $TIMEFILE -s ${CASE}_slice.ssf   $CASE`
fi
if [ -e ${CASE}_vslice.ssf ]; then
  time4=`$smvrepo/Utilities/Scripts/smokeview.sh $SMOKEVIEW -t $TIMEFILE -s ${CASE}_vslice.ssf  $CASE`
fi
echo $LABEL,$time0,$time1,$time2,$time3,$time4
if [ "$SETUP_XSERVER" == "1" ]; then
  source $smvrepo/Utilities/Scripts/stopXserver.sh >/dev/null 2>&1
fi
cd $CURDIR
