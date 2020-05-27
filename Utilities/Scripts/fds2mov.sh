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
    echo "-o - directory containing generated movie. [default: $OUTDIR]"
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
#                   wait_cases_end
#---------------------------------------------

wait_cases_end()
{
  while [[ `qstat -a | awk '{print $2 $4 $10}' | grep $(whoami) | grep ${JOBPREFIX} | grep -v 'C$'` != '' ]]; do
     JOBS_REMAINING=`qstat -a | awk '{print $2 $4 $10}' | grep $(whoami) | grep ${JOBPREFIX} | grep -v 'C$' | wc -l`
     echo "Waiting for ${JOBS_REMAINING} cases to complete."
     sleep 15
  done
}

#---------------------------------------------
#                  generate_images
#---------------------------------------------

generate_images_movie ()
{
while true; do
  echo ""
  echo "    processes: $NPROCS"
  echo "        queue: $QUEUE"
  echo "    smokeview: $SMOKEVIEW"
  echo "      qsmv.sh: $QSMV"
  echo " image script: $img_script"
  echo ""
  echo "p - define number of processes"
  echo "q - define queue"
  echo "1 - generate images"
  echo "2 - generate images and movie"
  echo "x - exit"
  read -p "option: " ans
  if [ "$ans" == "p" ]; then
    read -p "   enter number of processes: " NPROCS
    continue
  fi
  if [ "$ans" == "q" ]; then
    read -p "   enter queue: " QUEUE
    continue
  fi
  if [ "$ans" == "x" ]; then
    exit
  fi
  if [[ "$ans" -ge 1 ]] && [[ "$ans" -le "2" ]]; then
    if [ "$ans" == "1" ]; then
      GENERATE_IMAGES=1
      GENERATE_SCRIPTS $slice_index
      return
    fi
    if [ "$ans" == "2" ]; then
      GENERATE_IMAGES=1
      MAKE_MOVIE=1
      GENERATE_SCRIPTS $slice_index
      return
    fi
  fi
done
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

GENERATE_SCRIPTS ()
{
  ind=$1
  img_basename=${input}_slice_${ind}
  smv_scriptname=${img_basename}.ssf
  img_scriptname=${img_basename}.sh
  cat << EOF > ${smv_scriptname}
RENDERDIR
  $RENDERDIR
UNLOADALL
LOADSLICERENDER
EOF
  slice_quantity=`cat $slcffile | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2} }'`
  cat $slcffile | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2"\n" $3 $4} }' >> $smv_scriptname
  cat << EOF >> $smv_scriptname
  $img_basename 
  0 1
EOF
  echo ""
  cat << EOF > $img_scriptname
#!/bin/bash
NPROCS=$NPROCS
QUEUE=$QUEUE
SMOKEVIEW=$SMOKEVIEW
QSMV=$FIREMODELS/smv/Utilities/Scripts/qsmv.sh
\$QSMV -j $JOBPREFIX -P \$NPROCS -q \$QUEUE -e \$SMOKEVIEW -c $smv_scriptname $input
EOF
chmod +x $img_scriptname
}

#*** initialize variables

RENDERDIR=.
OUTDIR=.
NPROCS=1
QUEUE=batch
slice_index=
USE_INSTALLED=
HELP_ALL=
JOBPREFIX=SV_
GENERATE_IMAGES=
MAKE_MOVIE=

# define repo variables

CURDIR=`pwd`
SCRIPTDIR=`dirname "$0"`
cd $SCRIPTDIR/../../..
ROOTDIR=`pwd`
SMVREPO=$ROOTDIR/smv
cd $CURDIR
SMOKEVIEW=$SMVREPO/Build/smokeview/intel_linux_64/smokeview_linux_64
QSMV=$SMVREPO/Utilities/Scripts/qsmv.sh
MAKEMOVIE=$SMVREPO/Utilities/Scripts/make_movie.sh


#---------------------------------------------
#                  parse command line options 
#---------------------------------------------

while getopts 'e:hHio:p:q:r:' OPTION
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
  o)
   OUTDIR="$OPTARG"
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

generate_images_movie

if [ "$GENERATE_IMAGES" == "1" ]; then
  bash $img_scriptname
  if [ "$MAKE_MOVIE" == "1" ]; then
    wait_cases_end
    $MAKEMOVIE -o $OUTDIR $img_basename $img_basename
  fi
else
  echo ""
  echo "image generatingscript: $img_scriptnme"
  cat $img_scriptname
fi

