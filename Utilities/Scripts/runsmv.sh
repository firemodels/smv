#!/bin/bash

FED=
MOVIE=
RUNSCRIPT=
ssffile=
dummy=

while getopts 'Ad:fl:mn:p:t' OPTION
do
case $OPTION in
  A) # passthrough option
   ;;
  d)
   dir="$OPTARG"
   ;;
  f)
   FED="-fed"
   ;;
  l)
   dummy="$OPTARG"
   ;;
  m)
   MOVIE="y"
   ;;
  n)
   dummy="$OPTARG"
   ;;
  p)
   dummy="$OPTARG"
   ;;
  t)
   dummy=1
   ;;
esac
done
shift $(($OPTIND-1))

in=$1
in=${in%*.*}

if [ "$FED" == "" ]; then
  if [ "$MOVIE" == "" ]; then
    RUNSCRIPT=-runscript
    ssffile=$in.ssf
  else
    MOVIE=_movies
    RUNSCRIPT="-script $in$MOVIE.ssf"
    ssffile=$in$MOVIE.ssf
  fi
fi

# define BASEDIR if it doesn't exist
if [ "$BASEDIR" == "" ]; then
  BASEDIR=`pwd`
fi

#define SMV if it doesn't exist
CURDIR=`pwd`
if [ "$SMV" == "" ]; then
  SMV=../../Build/smokeview/intel_linux_64
  if [ -e $SMV ]; then
    cd $SMV
    SMV=`pwd`/smokeview_linux_64
    cd $CURDIR
    if [ ! -e $SMV ]; then
      SMV=
    fi
  fi
fi

#define SMVBINDIR if it doesn't exist
if [ "$SMVBINDIR" == "" ]; then
  SMVBINDIR=../../Build/for_bundle
  if [ -e $SMVBINDIR ]; then
    cd $SMVBINDIR
    SMVBINDIR=`pwd`
    cd $CURDIR
  fi
fi

fulldir=$BASEDIR/$dir
echo ""
echo "--- generating images for: $in.smv, `date`"

notfound=`$SMV -help 2>&1 | tail -1 | grep "not found" | wc -l`
if [ "$notfound" == "1" ];  then
  echo "*** Error: The program $SMV is not available. Run aborted."
  exit
fi
if ! [ -d $fulldir ]; then
  echo "*** Error: The directory $fulldir does not exist. Run aborted."
  exit
fi
if ! [ -e $fulldir/$in.smv ]; then
  echo "*** Error: The smokeview file, $fulldir/$in.smv, does not exist. Run aborted."
  exit
fi
if ! [ -e $fulldir/$ssffile ]; then
  echo "*** Error: The smokeview script file, $fulldir/$ssffile, does not exist. Run aborted."
  exit
fi

cd $fulldir
if [ "$SMVBINDIR" != "" ]; then
  SMVBINDIR="-bindir $SMVBINDIR"
fi
echo "$SMV $FED $SMVBINDIR -redirect $RUNSCRIPT $in"
      $SMV $FED $SMVBINDIR -redirect $RUNSCRIPT $in
