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
#                   restore_state
#---------------------------------------------

restore_state()
{
  if [ -e $GLOBALCONFIG ]; then
    source $GLOBALCONFIG
    HTMLDIR=$SLICE2HTML_HTMLDIR
    TIMEFRAME=$SLICE2HTML_TIMEFRAME
    EMAIL=$SLICE2HTML_EMAIL
  fi
}

#---------------------------------------------
#                   save_state
#---------------------------------------------

save_state()
{
  if [ -e $GLOBALCONFIG ]; then
    echo "#/bin/bash"                             >  $GLOBALCONFIG
    echo "export SLICE2HTML_HTMLDIR=$HTMLDIRDIR"  >> $GLOBALCONFIG
    echo "export SLICE2HTML_TIMEFRAME=$TIMEFRAME" >> $GLOBALCONFIG
    echo "export SLICE2HTML_EMAIL=$EMAIL"         >> $GLOBALCONFIG
  fi
}

#---------------------------------------------
#                  generate_html
#---------------------------------------------

generate_html ()
{
script_index=$1
while true; do
  echo ""
  echo "slice quantity: $slice_quantity "
  echo "     smokeview: $SMOKEVIEW"
  echo "         email: $EMAIL"
  echo ""
  echo "h - define directory containing html page"
  echo "m - select email address"
  echo "t - select time frame (-1 for all time frames)"
  echo "1 - generate html page"
  echo "x - exit"
  EXIT_SCRIPT=
  read -p "option: " ans
  if [ "$ans" == "h" ]; then
    read -p "   enter html directory: " HTMLDIR
    CHECK_WRITE $HTMLDIR
    continue
  fi
  if [ "$ans" == "m" ]; then
    read -p "   enter email address: " EMAIL
    continue
  fi
  if [ "$ans" == "t" ]; then
    read -p "   enter time frame (-1 for all frames): " TIMEFRAME
    continue
  fi
  if [ "$ans" == "x" ]; then
    EXIT_SCRIPT=1
    save_state
    exit
  fi
  if [ "$ans" == "1" ]; then
    save_state
    if [ -e $scriptname ]; then
      echo "***creating html file"
      scriptname=${SMVSCRIPTDIR}${input}_slice_${ans}.ssf
      rm -f $scriptname
      GENERATE_SCRIPT $script_index $scriptname
      echo dir=`pwd`
      echo "$SMOKEVIEW -htmlscript $scriptname -bindir $SMVBINDIR  $input"
            $SMOKEVIEW -htmlscript $scriptname -bindir $SMVBINDIR  $input
      if [ -e $htmlfile ]; then
        filesize=`ls -lk $htmlfile | awk '{print $5}'`
        echo "*** The html file, $htmlfile(${filesize}K), has been created."
      else
        echo "*** Creation of the html file $htmlfile failed."
      fi
    fi
    return
  fi
done
}

#---------------------------------------------
#                   GENERATE_SCRIPT
#---------------------------------------------

GENERATE_SCRIPT ()
{
  ind=$1
  scriptname=$2
  htmlbase=${input}_slice_${ind}
  htmlfile=$HTMLDIR/${htmlbase}.html
  cat << EOF > $scriptname
RENDERHTMLDIR
  $HTMLDIR
UNLOADALL
LOADSLICE
EOF
  cat $slicefilemenu | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2"\n" $3 $4"\n"} }' >> $scriptname
  slice_quantity=`cat $slicefilemenu | awk -v ind="$slice_index" -F"," '{ if($1 == ind){print $2} }'`
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
if [ ! -e $CONFIGDIR ]; then
  mkdir $CONFIGDIR
fi

GLOBALCONFIG=$CONFIGDIR/slice2html_global

SMVSCRIPTDIR=
touch test.$$ >& /dev/null
if [ -e test.$$ ]; then
  rm test.$$
else
  SMVSCRIPTDIR=${CONFIGDIR}/
fi

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
restore_state


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
  read -p "Select slice index to generate an html file: " ans
  if [[ "$ans" -ge 1 ]] && [[ "$ans" -le "$nslices" ]]; then
    slice_index=$ans
    slice_quantity=`cat $slicefilemenu | awk -v ind="$slice_index" -F"," '{ if($1 == ind){print $2} }'`
    generate_html $ans
    if [ "$EXIT_SCRIPT" != "" ]; then
      exit
    fi
    if [ "$EMAIL" != "" ]; then
      if [ -e $htmlfile ]; then
        echo "html file, $htmlfile, sent to $EMAIL"
        echo "" | mail -s "html page for of $slice_quantity" -a $htmlfile $EMAIL
      fi
    fi
  else
    echo index $ans out of bounds
  fi
done
