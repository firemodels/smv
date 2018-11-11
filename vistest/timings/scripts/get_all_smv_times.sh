#!/bin/bash
SMOKEVIEW=
LABEL=smokeview
TIMEFILE=timefile.$$
DIR=$HOME/SMVS/linux64

#---------------------------------------------
#                   usage
#---------------------------------------------

function usage {
echo "run smokeview to get timings"
echo ""
echo "Options:"
echo "-c - case to perform timings on"
echo "-d - directory containing smokeview versions {default: $DIR}"
echo "-h - display usage info"
if [ "$option" == "-H" ]; then
usage_all
fi
exit
}


while getopts 'c:d:h' OPTION
do
case $OPTION in
  d)
   DIR="$OPTARG"
   ;;
  h)
   usage
   ;;
esac
done
shift $(($OPTIND-1))

if [ ! -d $DIR ]; then
  echo "***fatal error: the directory $DIR does not exist"
  exit
fi

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
