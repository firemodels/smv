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
  echo "-p - number of processes [default $NPROCS]"
  echo "-q - queue  [default: $QUEUE]"
  echo "-h - show commonly used options"
  echo "-H - show all options"
  if [ "$HELP_ALL" == "1" ]; then
    echo "-e - full path of smokeview executable."
    echo "     [default: $SMOKEVIEW]"
    echo "-i - use installed smokeview"
    echo "-r - directory containing generated images. [default: $RENDERDIR]"
  fi
  echo ""
  exit
}

#---------------------------------------------
#                   is_file_installed
#---------------------------------------------

get_smokeview ()
{
  if [ "$USE_INSTALLED" == "" ]; then
    if [ -e $SMOKEVIEW ]; then
      return 0
    fi
    echo "***error: The smokeview file, $SMOKEVIEW, does not exist."
    return 1
  else
    out=/tmp/program.out.$$
    smokeview -v >& $out
    notfound=`cat $out | tail -1 | grep "not found" | wc -l`
    rm $out
    if [ "$notfound" == "1" ] ; then
      echo "***error: smokeview is not installed. Install smokeview"
      echo "          and/or add smokeview to your PATH" 
      return 1
    fi
    SMOKEVIEW=`which smokeview`
  fi
  return 0
}

#---------------------------------------------
#                   OUTPUT_SLICES
#---------------------------------------------

OUTPUT_SLICES ()
{
  cat $slcffile | awk -F"," '{ print $1" ",$2," ",$3," ",$4}'
}

#---------------------------------------------
#                   select_slice_file
#---------------------------------------------

select_slice_file ()
{
while true; do
  OUTPUT_SLICES
  read -p "Select slice file: " ans
  if [[ "$ans" -ge 1 ]] && [[ "$ans" -le "$nslices" ]]; then
    slice_index=$ans
    return 0
  else
    echo index $ans out of bounds
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
  basename=${input}_slice_${ind}
  scriptname=${basename}.ssf
  makemovie=${basename}.sh
  cat << EOF > $scriptname
RENDERDIR
  $RENDERDIR
UNLOADALL
LOADSLICERENDER
EOF
  slice_quantity=`cat $slcffile | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2} }'`
  cat $slcffile | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2"\n" $3 $4} }' >> $scriptname
  cat << EOF >> $scriptname
  $basename 
  0 1
EOF
  echo ""
  echo "image generating script: $makemovie "
  cat << EOF > $makemovie
#!/bin/bash
NPROCS=$NPROCS
QUEUE=$QUEUE
SMOKEVIEW=$SMOKEVIEW
QSMV=$FIREMODELS/smv/Utilities/Scripts/qsmv.sh
\$QSMV -P \$NPROCS -q \$QUEUE -e \$SMOKEVIEW -c $scriptname $input
EOF
chmod +x $makemovie
}

#*** initialize variables

RENDERDIR=.
NPROCS=1
QUEUE=batch
slice_index=
USE_INSTALLED=
HELP_ALL=

# define repo variables

CURDIR=`pwd`
SCRIPTDIR=`dirname "$0"`
cd $SCRIPTDIR/../../..
ROOTDIR=`pwd`
cd $CURDIR
SMOKEVIEW=$ROOTDIR/smv/Build/smokeview/intel_linux_64/smokeview_linux_64
SMOKEVIEW_TEST=$ROOTDIR/smv/Build/smokeview/intel_linux_64/smokeview_linux_test_64
QSMV=$ROOTDIR/smv/Utilities/Scripts/qsmv.sh


#---------------------------------------------
#                  parse command line options 
#---------------------------------------------

while getopts 'e:hHip:q:r:' OPTION
do
case $OPTION  in
  e)
   SMOKEVIEW="$OPTARG"
   ;;
  h)
   Usage
   exit
   ;;
  H)
   HELP_ALL=1
   Usage
   exit
   ;;
  i)
   USE_INSTALLED=1
   ;;
  p)
   NPROCS="$OPTARG"
   ;;
  q)
   QUEUE="$OPTARG"
   ;;
  r)
   RENDERDIR="$OPTARG"
   ;;
esac
done
shift $(($OPTIND-1))

input=$1

smvfile=$1.smv
slcffile=$1.slcf

get_smokeview || exit 1

if [ ! -e $smvfile ]; then
  echo "***error: $smvfile does not exist"
  exit
fi

if [ ! -e $slcffile ]; then
  $SMOKEVIEW -slice_info $input >& /dev/null
fi

if [ ! -e $slcffile ]; then
  echo "*** error: $slcffile does not exist"
  exit
fi

nslices=`cat $slcffile | wc -l`
if [ "$nslices" == "0" ]; then
  echo "*** error:  No slice files found in $smvfile"
  exit
fi

select_slice_file

if [ "$slice_index" != "" ]; then
  scriptname=${input}_slice_${slice_index}.ssf
  rm -f $scriptname
  GENERATE_SCRIPT $slice_index $scriptname
fi


