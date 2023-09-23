#!/bin/bash

# ---------------------------- usage ----------------------------------

function usage {
  echo "Usage: background.sh"
  echo ""
  echo "runs background prog"
  echo ""
  echo "options:"
  echo " -b background - patth for background program"
  echo " -d dir - specify directory where the case is found [default: .]"
  echo ""
  exit
}

CURDIR=`pwd`

#*** define repo root

QSMV=$0
QSMV_PATH=$(dirname `which $0`)
cd $QSMV_PATH/../../..
REPOROOT=`pwd`

cd $CURDIR

if [ $# -lt 1 ]; then
  usage
fi

commandline=`echo $* | sed 's/-V//' | sed 's/-v//'`

DIR=
EXE=
USE_FULL=
nprocs=1
BACKGROUND=background

#*** read in parameters from command line

while getopts 'Ab:d:e:hIn:p:q:tv' OPTION
do
case $OPTION  in
  A)
   dummy=
   ;;
  b)
   BACKGROUND="$OPTARG"
   ;;
  d)
   DIR="$OPTARG"
   ;;
  e)
   EXE="$OPTARG"
   ;;
  h)
   usage
   exit
   ;;
  I)
   USE_FULL=1
   ;;
  n)
   dummy="${OPTARG}"
   ;;
  p)
   nprocs="${OPTARG}"
   ;;
  q)
   dummy="${OPTARG}"
   ;;
  t)
   dummy=
   ;;
  v)
   showinput=1
   v_arg="-v"
   ;;
esac
done
shift $(($OPTIND-1))

if [ "$EXE" == "" ]; then
  echo "***error: -e parameter required"
  exit
fi

#*** define input file

in=$1
infile=${in%.*}
if [ "$USE_FULL" == "1" ]; then
  input=$in
else
  input=$infile
fi

if [ "$DIR" != "" ]; then
  cd $DIR
fi
if [ "$STOPFDSMAXITER" != "" ]; then
  echo $STOPFDSMAXITER > ${infile}.stop
else
  rm -f ${infile}.stop
fi

MPIEXEC=
#ncores=`grep processor /proc/cpuinfo | wc -l`
#if [[ $nprocs -gt 1 ]] && [[ $ncores -ge $nprocs ]] && [[ "`uname`" != "Darwin" ]]; then
#  MPIEXEC="mpiexec -n $nprocs "
#fi
#echo $MPIEXEC
$BACKGROUND -d 2 -u 75 $MPIEXEC $EXE $input
