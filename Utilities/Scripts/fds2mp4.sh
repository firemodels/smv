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
#                   OUTPUT_VIEWPOINTS
#---------------------------------------------

OUTPUT_VIEWPOINTS ()
{
  cat $viewpointmenu | awk -F"," '{ print $1" ",$2}'
}

#---------------------------------------------
#                   CHECK_WRITE
#---------------------------------------------

CHECK_WRITE ()
{
  DIR=$1
  if [ ! -e $DIR ]; then
    mkdir $DIR
    if [ ! -e $DIR ]; then
      echo "***error: the directory $DIR could not be created"
      return 1
    fi
  fi
  touch $DIR/.test
  if [ ! -e $DIR/.test ]; then
    echo "***error: the directdory $DIR cannot be written too"
    return 1
  fi
  rm $DIR/.test
}


#---------------------------------------------
#                   OUTPUT_SLICES
#---------------------------------------------

OUTPUT_SLICES ()
{
  cat $slicefilemenu | awk -F"," '{ print $1" ",$2," ",$3," ",$4}'
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
#                   restore_state
#---------------------------------------------

restore_state()
{
  if [ -e $GLOBALCONFIG ]; then
    source $GLOBALCONFIG
    NPROCS=${FDS2MOV_NPROCS}
    QUEUE=${FDS2MOV_QUEUE}
    RENDERDIR=${FDS2MOV_RENDERDIR}
    MOVIEDIR=${FDS2MOV_MOVIEDIR}
    EMAIL=${FDS2MOV_EMAIL}
  fi
  LOCALCONFIG=$CONFIGDIR/fds2mp4_${input}
  if [ -e $LOCALCONFIG ]; then
    source $LOCALCONFIG
    viewpoint=$FDS2MOV_VIEWPOINT
  fi
}

#---------------------------------------------
#                   save_state
#---------------------------------------------

save_state()
{
  echo "#/bin/bash"                           >  $GLOBALCONFIG
  echo "export FDS2MOV_NPROCS=$NPROCS"        >> $GLOBALCONFIG
  echo "export FDS2MOV_QUEUE=$QUEUE"          >> $GLOBALCONFIG
  echo "export FDS2MOV_RENDERDIR=$RENDERDIR"  >> $GLOBALCONFIG
  echo "export FDS2MOV_MOVIEDIR=$MOVIEDIR"    >> $GLOBALCONFIG
  echo "export FDS2MOV_EMAIL=$EMAIL"          >> $GLOBALCONFIG
  
  LOCALCONFIG=$CONFIGDIR/fds2mp4_${input}
  echo "#/bin/bash"                               >  $LOCALCONFIG
  echo "export FDS2MOV_VIEWPOINT=\"$viewpoint\""  >> $LOCALCONFIG
}

#---------------------------------------------
#                  generate_images
#---------------------------------------------

generate_images_movie ()
{
while true; do
  echo ""
  echo "slice quantity: $slice_quantity "
  echo "     processes: $NPROCS"
  echo "         queue: $QUEUE"
  echo "       mp4 dir: $MOVIEDIR"
  echo "       PNG dir: $RENDERDIR"
  echo "     smokeview: $SMOKEVIEW"
  echo "       qsmv.sh: $QSMV"
  echo "     viewpoint: $viewpoint"
  echo "  image script: $img_scriptname"
  echo "         email: $EMAIL"
  echo ""
  echo "a - define directory containing animation"
  echo "p - define number of processes"
  echo "q - define queue"
  echo "r - define directory containing rendered images"
  echo "v - select viewpoint"
  echo "m - select emaail address"
  echo "1 - generate PNG images"
  echo "2 - generate PNG images and an MP4 animation"
  echo "x - exit"
  read -p "option: " ans
  if [ "$ans" == "a" ]; then
    read -p "   enter animation directory: " MOVIEDIR
    CHECK_WRITE $MOVIEDIR
    continue
  fi
  if [ "$ans" == "i" ]; then
    read -p "   enter image frame directory: " RENDERDIR
    CHECK_WRITE $RENDERDIR
    continue
  fi
  if [ "$ans" == "m" ]; then
    read -p "   enter email address: " EMAIL
    continue
  fi
  if [ "$ans" == "p" ]; then
    read -p "   enter number of processes: " NPROCS
    continue
  fi
  if [ "$ans" == "q" ]; then
    read -p "   enter queue: " QUEUE
    continue
  fi
  if [ "$ans" == "v" ]; then
    select_viewpoint
    continue
  fi
  if [ "$ans" == "x" ]; then
    save_state
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
#                   select_viewpoint
#---------------------------------------------

select_viewpoint ()
{
while true; do
  OUTPUT_VIEWPOINTS
  read -p "Select viewpoint: " ans
  if [ "$ans" == "d" ]; then
    viewpoint=
    return 0
  fi
  re='^[0-9]+$'
  if ! [[ $ans =~ $re ]]; then
    echo "***error: $ans is an invalid selection"
    continue
  fi
  if [[ $ans -ge 1 ]] && [[ $ans -le $nviewpoints ]]; then
    viewpoint_index=$ans
    viewpoint=`cat $viewpointmenu | awk -v ind="$viewpoint_index" -F"," '{ if($1 == ind){print $2} }'`
    return 0
  else
    echo index $ans out of bounds
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
  re='^[0-9]+$'
  read -p "Select slice file: " ans
  if ! [[ $ans =~ $re ]]; then
    echo "***error: $ans is an invalid selection"
    continue
  fi
  if [[ "$ans" -ge 1 ]] && [[ "$ans" -le "$nslices" ]]; then
    slice_index=$ans
    img_basename=${input}_slice_${slice_index}
    smv_scriptname=${img_basename}.ssf
    img_scriptname=${img_basename}.sh
    slice_quantity=`cat $slicefilemenu | awk -v ind="$slice_index" -F"," '{ if($1 == ind){print $2} }'`
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
  cat << EOF > ${smv_scriptname}
RENDERDIR
  $RENDERDIR
UNLOADALL
EOF
if [ "$viewpoint" != "" ]; then
  cat << EOF >> ${smv_scriptname}
SETVIEWPOINT
  $viewpoint
EOF
fi
  cat << EOF >> ${smv_scriptname}
LOADSLICERENDER
EOF
  slice_quantity=`cat $slicefilemenu | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2} }'`
  cat $slicefilemenu | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2"\n" $3 $4} }' >> $smv_scriptname
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
MOVIEDIR=/var/www/html/`whoami`
if [ ! -e $MOVIEDIR ]; then
  MOVIEDIR=.
fi
NPROCS=20
QUEUE=batch4
slice_index=
USE_INSTALLED=
HELP_ALL=
JOBPREFIX=SV_
GENERATE_IMAGES=
MAKE_MOVIE=

CONFIGDIR=$HOME/.fds2mp4
if [ ! -e $CONFIGDIR ]; then
  mkdir $CONFIGDIR
fi
GLOBALCONFIG=$CONFIGDIR/fds2mp4_global

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
EMAIL=


#---------------------------------------------
#                  parse command line options 
#---------------------------------------------

while getopts 'e:hi' OPTION
do
case $OPTION  in
  e)
   SMOKEVIEW="$OPTARG"
   ;;
  h)
   Usage
   exit
   ;;
  i)
   USE_INSTALLED=1
   ;;
esac
done
shift $(($OPTIND-1))

input=$1
restore_state

smvfile=$1.smv
slicefilemenu=$HOME/.fds2mp4/$1.slcf


get_smokeview || exit 1

if [ ! -e $smvfile ]; then
  echo "***error: $smvfile does not exist"
  exit
fi

$SMOKEVIEW -info $input >& /dev/null

# get viewpoint menu (optional)

nviewpoints=0
viewpointmenu=$HOME/.fds2mp4/$1.viewpoints
if [ -e $viewpointmenu ]; then
  nviewpoints=`cat $viewpointmenu | wc -l`
  (( nviewpoints -= 3 ))
fi

# get slice file menu (required)

if [ ! -e $slicefilemenu ]; then
  echo "*** error: $slicefilemenu does not exist"
  exit
fi

nslices=`cat $slicefilemenu | wc -l`
(( nslices -= 2 ))
if [ $nslices  -eq 0 ]; then
  echo "*** error:  No slice files were found in $smvfile"
  exit
fi

select_slice_file

generate_images_movie

save_state

if [ "$GENERATE_IMAGES" == "1" ]; then
  bash $img_scriptname
  if [ "$MAKE_MOVIE" == "1" ]; then
    wait_cases_end
    animation_file=$MOVIEDIR/${img_basename}.mp4
    $MAKEMOVIE -i $RENDERDIR -o $MOVIEDIR $img_basename $img_basename
    if [ "$EMAIL" != "" ]; then
      if [ -e $animation_file ]; then
        echo "animation file, $animation_file, sent to $EMAIL"
        echo "" | mail -s "animation of $slice_quantity" -a $animation_file $EMAIL
      fi
    fi
  fi
else
  echo ""
  echo "image generatingscript: $img_scriptnme"
  cat $img_scriptname
fi

