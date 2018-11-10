#!/bin/bash
SMOKEVIEW=
LABEL=smokeview
SETUP_XSERVER=1
CASE=plume_timing 
TIMEFILE=timefile.$$
DIR=$HOME/SMVS

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
cd $CURDIR

source $smvrepo/Utilities/Scripts/startXserver.sh >/dev/null 2>&1
for dir in $DIR/SMV*
do
SMOKEVIEW=$dir/bin/smokeview
if [ -e $SMOKEVIEW ]; then
  LABEL=`basename $dir`
  cd $CURDIR
  ./get_smv_times.sh -n -e $SMOKEVIEW -l $LABEL
fi
done
source $smvrepo/Utilities/Scripts/stopXserver.sh >/dev/null 2>&1
cd $CURDIR
