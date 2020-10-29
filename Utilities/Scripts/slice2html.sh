#!/bin/bash

#---------------------------------------------
#                   Usge
#---------------------------------------------

function Usage {
  scriptname=`basename $0`
  echo "Usage: $scriptname [options] casename"
  echo ""
  echo "This script generates image frames for a specified slice file"
  echo ""
  echo "-e path - full path of smokeview executable."
  echo "     [default: $SMOKEVIEW]"
  echo "-h - show this mesage"
#  echo "-i - use installed smokeview"
  exit
}

#---------------------------------------------
#                   is_smokeview_installed
#---------------------------------------------

is_smokeview_installed()
{
  out=/tmp/program.out.$$
  smokeview -v >& $out
  notfound=`cat $out | tail -1 | grep "not found" | wc -l`
  rm $out
  if [ "$notfound" == "1" ] ; then
    echo "***error: $program not installed"
    return 1
  fi
  return 0
}

#---------------------------------------------
#                   OUTPUT_SLICES
#---------------------------------------------

OUTPUT_SLICES ()
{
  cat $slicefilemenu | awk -F"," '{ print $1" ",$2," ",$3," ",$4}'
}

#---------------------------------------------
#                   GENERATE_SCRIPT
#---------------------------------------------

GENERATE_SCRIPT ()
{
  ind=$1
  scriptname=$2
  scriptname=${input}_slice_${ind}.ssf
  htmlbase=${input}_slice_${ind}
  htmlfile=$HTMLDIR/${htmlbase}.html
  cat << EOF > $scriptname
RENDERHTMLDIR
  $HTMLDIR
UNLOADALL
LOADSLICE
EOF
  cat $slicefilemenu | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2"\n" $3 $4"\n"} }' >> $scriptname
  cat << EOF >> $scriptname
RENDERHTMLALL
  $htmlbase
EOF
}

#----------------------------- script begins ---------------------------------------

# define repo variables

CURDIR=`pwd`
SCRIPTDIR=`dirname "$0"`
cd $SCRIPTDIR/../../..
ROOTDIR=`pwd`
SMVREPO=$ROOTDIR/smv
BOTREPO=$ROOTDIR/bot
cd $CURDIR
SMOKEVIEW=$SMVREPO/Build/smokeview/intel_linux_64/smokeview_linux_64
SMVBINDIR=$BOTREPO/Bundle/smv/for_bundle
CONFIGDIR=$HOME/.smokeview

#---------------------------------------------
#                  parse command line options 
#---------------------------------------------

while getopts 'e:hi' OPTION
do
case $OPTION  in
  e)
   SMOKEVIEW="$OPTARG"
   if [ ! -e $SMOKEVIEW ]; then
     echo "***error: smokeview not found at $SMOKEVIEW"
   fi
   ;;
  h)
   Usage
   exit
   ;;
  i)
   is_smokeview_installed || exit 1
   SMOKEVIEW=`which smokeview`
   ;;
esac
done
shift $(($OPTIND-1))


if [ ! -e $SMOKEVIEW ]; then
  echo "***error: smokeview not found at $SMOKEVIEW"
  exit 1
fi

input=$1

smvfile=$1.smv
slicefilemenu=$CONFIGDIR/$1.slcf

HTMLDIR=/var/www/html/`whoami`
if [ ! -e $HTMLDIR ]; then
  HTMLDIR=.
fi

if [ ! -e $smvfile ]; then
  echo "***error: The .smv file, $smvfile, does not exist"
fi

if [ ! -e $slicefilemenu ]; then
  $SMOKEVIEW -bindir $SMVBINDIR -info $input >& /dev/null
fi

if [ ! -e $slicefilemenu ]; then
  echo "*** error: The slice menu file, $slicefilemenu, does not exist"
  exit 1
fi

nslices=`cat $slicefilemenu | wc -l`
if [ "$nslices" == "0" ]; then
  echo "*** No slice files exist in the smokeview file $smvfile"
  exit
fi

while true; do
  OUTPUT_SLICES
  echo "x - exit script"
  read -p "Select slice index to generate an html file: " ans
  if [ "$ans" == "x" ]; then
    exit 
  fi
  if [ "$ans" == "r" ]; then
    echo continue 
  fi
  if [[ "$ans" -ge 1 ]] && [[ "$ans" -le "$nslices" ]]; then
    echo converting index $ans
    scriptname=${input}_slice_${ans}.ssf
    rm -f $scriptname
    GENERATE_SCRIPT $ans $scriptname
    if [ -e $scriptname ]; then
      echo "***creating html file"
      rm -f $htmlfile
      echo dir=`pwd`
      echo "$SMOKEVIEW -htmlscript $scriptname -bindir $SMVBINDIR  $input"
            $SMOKEVIEW -htmlscript $scriptname -bindir $SMVBINDIR  $input
      if [ -e $htmlfile ]; then
        filesize=`ls -lk $htmlfile | awk '{print $5}'`
        echo "*** The html file, $htmlfile(${filesize}K), has been created."
      else
        echo "*** The html file $htmlfile failed to be created."
      fi
    fi
  else
    echo index $ans out of bounds
  fi
done

