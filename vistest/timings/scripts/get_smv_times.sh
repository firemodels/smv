#!/bin/bash
SMOKEVIEW=
LABEL=smokeview
SETUP_XSERVER=1
CASE=plume_timing 
TIMEFILE=timefile.$$

while getopts 'c:e:l:n' OPTION
do
case $OPTION in
  c)
   CASE="$OPTARG"
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

CURDIR=`pwd`
cd ../../../
smvrepo=`pwd`
cd $CURDIR/../cases
if [ "$SETUP_XSERVER" == "1" ]; then
  source $smvrepo/Utilities/Scripts/startXserver.sh >/dev/null 2>&1
fi
time0=`$smvrepo/Utilities/Scripts/smokeview.sh $SMOKEVIEW -t $TIMEFILE -s ${CASE}_startup.ssf $CASE`
time1=`$smvrepo/Utilities/Scripts/smokeview.sh $SMOKEVIEW -t $TIMEFILE -s ${CASE}_iso.ssf     $CASE`
time2=`$smvrepo/Utilities/Scripts/smokeview.sh $SMOKEVIEW -t $TIMEFILE -s ${CASE}_3dsmoke.ssf $CASE`
time3=`$smvrepo/Utilities/Scripts/smokeview.sh $SMOKEVIEW -t $TIMEFILE -s ${CASE}_slice.ssf   $CASE`
time4=`$smvrepo/Utilities/Scripts/smokeview.sh $SMOKEVIEW -t $TIMEFILE -s ${CASE}_vslice.ssf  $CASE`
echo $LABEL,$time0,$time1,$time2,$time3,$time4
if [ "$SETUP_XSERVER" == "1" ]; then
  source $smvrepo/Utilities/Scripts/stopXserver.sh >/dev/null 2>&1
fi
cd $CURDIR
