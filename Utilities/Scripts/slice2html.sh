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
#                   trim
#---------------------------------------------

trim()
{
  local var="$*"
# remove leading whitespace characters
  var="${var#"${var%%[![:space:]]*}"}"
# remove trailing whitespace characters
  var="${var%"${var##*[![:space:]]}"}"
  printf '%s' "$var"
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
#                   restore_localstate
#---------------------------------------------

restore_localstate()
{
  index=$1
  LOCALCONFIG=$CONFIGDIR/slice2html_${input}_${index}
  if [ -e $LOCALCONFIG ]; then
    source $LOCALCONFIG
    var="SLICE2HTML_${input}_${index}_min"
    valmin=${!var}
    var="SLICE2HTML_${input}_${index}_max"
    valmax=${!var}
    have_bounds=1
  fi
}

#---------------------------------------------
#                   restore_state
#---------------------------------------------

restore_state()
{
  if [ -e $GLOBALCONFIG ]; then
    source $GLOBALCONFIG
    HTMLDIR=${SLICE2HTML_HTMLDIR}
    TIMEFRAME=${SLICE2HTML_TIMEFRAME}
    EMAIL=${SLICE2HTML_EMAIL}
  fi
}

#---------------------------------------------
#                   save_localstate
#---------------------------------------------

save_localstate()
{
  index=$1
  LOCALCONFIG=$CONFIGDIR/slice2html_${input}_${index}
  echo "#/bin/bash"                                 > $LOCALCONFIG
  echo "SLICE2HTML_${input}_${index}_min=$valmin"  >> $LOCALCONFIG
  echo "SLICE2HTML_${input}_${index}_max=$valmax"  >> $LOCALCONFIG
}

#---------------------------------------------
#                   save_state
#---------------------------------------------

save_state()
{
  echo "#/bin/bash"                      >  $GLOBALCONFIG
  echo "SLICE2HTML_HTMLDIR=$HTMLDIRDIR"  >> $GLOBALCONFIG
  echo "SLICE2HTML_TIMEFRAME=$TIMEFRAME" >> $GLOBALCONFIG
  echo "SLICE2HTML_EMAIL=$EMAIL"         >> $GLOBALCONFIG
}

#---------------------------------------------
#                   writeini
#---------------------------------------------


writeini ()
{
ini_filename=$1
echo "valmin=$valmin"
if [ "$valmin" != "" ]; then
cat << EOF > $ini_filename
V2_SLICE
 0 $valmin 0 $valmax $slice_quantity_short

EOF
fi
}


#---------------------------------------------
#                   select_slice
#---------------------------------------------

select_slice()
{
    have_bounds=
    cat $slicefilemenu | awk -F"," '{ print $1" ",$2," ",$3," ",$4}'
    read -p "Select slice index to generate an html file: " ans
    if [[ "$ans" -ge 1 ]] && [[ "$ans" -le "$nslices" ]]; then
      slice_index=$ans
      slice_quantity=`cat $slicefilemenu | awk -v ind="$slice_index" -F"," '{ if($1 == ind){print $2} }'`
      slice_quantity=`trim "$slice_quantity"`

      slice_quantity_short=`grep -A 4 SLCF $smvfile | grep "$slice_quantity" -A 1 | head -2 | tail -1`
      slice_quantity_short=`trim "$slice_quantity_short"`

      slice_quantity_unit=`grep -A 4 SLCF $smvfile | grep "$slice_quantity" -A 2 | tail -1`
      slice_quantity_unit=`trim "$slice_quantity_unit"`
      restore_localstate $ans

    else
      echo slice index, $ans, is out of bounds
    fi
}

#---------------------------------------------
#                  generate_html
#---------------------------------------------

generate_html ()
{
while true; do
  echo ""
  echo "slice quantity: $slice_quantity "
  if [ "$TIMEFRAME" == "-1" ]; then
    echo " time frame(s):  all"
  else
    echo " time frame(s):  $TIMEFRAME"
  fi
if [ "$have_bounds" == "1" ]; then
  echo "      min, max: $valmin $slice_quantity_unit, $valmax $slice_quantity_unit"
else
  echo "        bounds: default"
fi

  echo "      html dir: $HTMLDIR"
  echo "     smokeview: $SMOKEVIEW"
  echo "         email: $EMAIL"
  echo ""
  echo "s - select slice"
  echo "b - set bounds"
  echo "h - set html directory"
  echo "m - set email address"
  echo "t - set time frame (-1 for all time frames)"
  echo "1 - generate html page"
  echo "x - exit"

  read -p "option: " ans
  if [ "$ans" == "h" ]; then
    read -p "   enter html dir: " HTMLDIR
    CHECK_WRITE $HTMLDIR
    continue
  fi
  if [ "$ans" == "b" ]; then
    have_bounds=1
    read -p "   set $slice_quantity_short min: " valmin
    read -p "   set $slice_quantity_short max: " valmax
    save_localstate $slice_index
    continue
  fi
  if [ "$ans" == "s" ]; then
    select_slice
    continue
  fi
  if [ "$ans" == "m" ]; then
    read -p "   set email address: " EMAIL
    continue
  fi
  if [ "$ans" == "t" ]; then
    read -p "   enter time frame (-1 for all frames): " TIMEFRAME
    continue
  fi
  if [ "$ans" == "x" ]; then
    save_state
    exit
  fi
  if [ "$ans" == "1" ]; then
    save_state
    if [ -e $scriptname ]; then
      echo "***creating html file"
      scriptname=${SMVSCRIPTDIR}${input}_slice_${slice_index}.ssf
      smv_inifilename=${SMVSCRIPTDIR}${input}_slice_${slice_index}.ini
      writeini $smv_inifilename
      rm -f $scriptname
      GENERATE_SCRIPT $slice_index $scriptname $smv_inifilename
      echo dir=`pwd`
      echo "$SMOKEVIEW -htmlscript $scriptname -bindir $SMVBINDIR  $input"
            $SMOKEVIEW -htmlscript $scriptname -bindir $SMVBINDIR  $input
      if [ -e $htmlfile ]; then
        filesize=`ls -lk $htmlfile | awk '{print $5}'`
        filesize=$((filesize/1000))
        echo "*** The html file, $htmlfile(${filesize}K), has been created."
        if [[ "$EMAIL" != "" ]] && [[ -e $htmlfile ]]; then
          echo "*** The html file, $htmlfile(${filesize}K), has been emailed to $EMAIL"
          echo "" | mail -s "html page for $slice_quantity(${filesize}K)" -a $htmlfile $EMAIL
        fi
      else
        echo "*** Creation of the html file $htmlfile failed."
      fi
    fi
    continue
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
  ini_filename=$3
  htmlbase=${input}_slice_${ind}
  htmlfile=$HTMLDIR/${htmlbase}.html
  cat << EOF > $scriptname
RENDERHTMLDIR
  $HTMLDIR
UNLOADALL
LOADINIFILE  
 $ini_filename  
LOADSLICE
EOF
  cat $slicefilemenu | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2"\n" $3 $4"\n"} }' >> $scriptname

  slice_quantity=`cat $slicefilemenu | awk -v ind="$slice_index" -F"," '{ if($1 == ind){print $2} }'`
  slice_quantity=`trim "$slice_quantity"`

  slice_quantity_short=`grep -A 4 SLCF $slicefilemenu | grep "$slice_quantity" -A 1 | head -2 | tail -1`
  slice_quantity_short=`trim "$slice_quantity_short"`

  slice_quantity_unit=`grep -A 4 SLCF $slicefilemenu | grep "$slice_quantity" -A 2 | tail -1`
  slice_quantity_unit=`trim "$slice_quantity_unit"`

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
if [ ! -e $SMOKEVIEW ]; then
  SMOKEVIEW=$SMVREPO/Build/smokeview/intel_linux_64/smokeview_linux_test_64
fi

SMVBINDIR=$SMVREPO/Build/for_bundle

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
  exit 1
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
  echo "*** No slice files found in the smokeview file $smvfile"
  exit
fi

select_slice
generate_html
