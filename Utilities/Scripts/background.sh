#!/bin/bash

# ---------------------------- usage ----------------------------------

function usage {
  echo "Usage: background.sh"
  echo ""
  echo "runs background prog"
  echo ""
  echo "options:"
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

#*** read in parameters from command line

while getopts 'd:hp:v' OPTION
do
case $OPTION  in
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
  p)
   dummy="${OPTARG}"
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

if [ "$DIR" != "" ]; then
  cd $DIR
fi
background -u 50 $EXE $infile
 
