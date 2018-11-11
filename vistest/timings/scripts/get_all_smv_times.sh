#!/bin/bash
SMOKEVIEW=
LABEL=smokeview
TIMEFILE=timefile.$$
CASE=
CASEDIR=

platform="linux"
if [ "`uname`" == "Darwin" ]
then
  platform="osx"
fi

SMVDIR=$HOME/SMVS/linux64
if [ "$platform" == "osx" ]; then
  SMVDIR=$HOME/SMVS/osx64
fi

#---------------------------------------------
#                   usage
#---------------------------------------------

function usage {
echo "run smokeview to get timings"
echo ""
echo "Options:"
echo "-c - case to perform timings on"
echo "-C - directory containing case"
echo "-d - directory containing smokeview versions {default: $SMVDIR}"
echo "-h - display usage info"
if [ "$option" == "-H" ]; then
usage_all
fi
exit
}

while getopts 'c:C:d:h' OPTION
do
case $OPTION in
  c)
   CASE="$OPTARG"
   ;;
  C)
   CASEDIR="$OPTARG"
   ;;
  d)
   SMVDIR="$OPTARG"
   ;;
  h)
   usage
   ;;
esac
done
shift $(($OPTIND-1))

if [ ! -d $SMVDIR ]; then
  echo "***fatal error: the directory $SMVDIR does not exist"
  exit
fi

if [ "$CASE" != "" ]; then
  CASE="-c $CASE"
fi
if [ "$CASEDIR" != "" ]; then
  CASEDIR="-C $CASEDIR"
fi

CURDIR=`pwd`
cd ../../../
smvrepo=`pwd`
cd $CURDIR

if [ "$platform" == "linux" ]; then
  source $smvrepo/Utilities/Scripts/startXserver.sh >/dev/null 2>&1
fi
for dir in $SMVDIR/SMV*
do
SMOKEVIEW=$dir/bin/smokeview
if [ -e $SMOKEVIEW ]; then
  LABEL=`basename $dir`
  cd $CURDIR
  ./get_smv_times.sh -n -e $SMOKEVIEW -l $LABEL $CASE $CASEDIR
fi
done
if [ "$platform" == "linux" ]; then
  source $smvrepo/Utilities/Scripts/stopXserver.sh >/dev/null 2>&1
fi
cd $CURDIR
